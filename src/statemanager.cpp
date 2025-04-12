#include "statemanager.hpp"

#include "helpers.hpp"
#include "servo/interfaces/group/servo.hpp"

#include <ranges>

static const double posinside{-50.}, posoutside{0.};
State::State(uint32_t pos, std::shared_ptr<DisplayIf> display,
             std::shared_ptr<LedrgdIf> ledrgb,
             std::shared_ptr<servo::ServoIf> servo,
             std::shared_ptr<graphs::GraphIf> graph,
             std::shared_ptr<graphs::helpers::TimeMonitor> tm,
             std::shared_ptr<logs::LogIf> logif) :
    pos{pos}, logif{logif}, display{display}, ledrgb{ledrgb}, servo{servo},
    graph{graph}, tm{tm}
{
    auto timems{str(tm->getmiliseconds())};
    graph->add(timems + separator + str(pos));
    log(logif, logs::level::debug, "Created state @ pos: " + str(pos));
    run();
}

State::~State()
{
    if (servo)
        servo->moveto(posoutside);
    if (pos)
        ledrgb->light(pos - 1, ledrgb::colortype::red);
    auto timems{str(tm->getmiliseconds())};
    graph->add(timems + separator + str(pos));
    log(logif, logs::level::debug, "Removed state @ pos: " + str(pos));
}

void State::run()
{
    if (servo)
        servo->moveto(posinside);
    if (pos)
        ledrgb->light(pos - 1, ledrgb::colortype::green);
    display->show(str(pos), !pos ? disptype::colortype::second
                                 : disptype::colortype::first);
}

StateManager::StateManager(std::shared_ptr<logs::LogIf> logif) : logif{logif}
{
    adc = adc::Factory::create<adc::rpi::ads1115::Adc,
                               adc::rpi::ads1115::configevt_t>(
        {"iio:device0", 0, 3.3, {}, {}});
    pwm = pwm::Factory::create<pwm::rpi::pca9685::Pwm,
                               pwm::rpi::pca9685::config_t>(
        {4,
         90,
         100,
         pwm::rpi::pca9685::polaritytype::normal,
         "/sys/class/pwm/pwmchip2",
         {}});
    adcobserver = helpers::Observer<adc::ObsData>::create(
        [this](const adc::ObsData& data) {
            auto [volt, perc] = std::get<1>(data);
            pwm->setduty(perc);
        });
    adc->observe(adcobserver);

    display = display::Factory::create<disptype::Display, disptype::config_t,
                                       disptype::param_t>(
        "/dev/spidev0.0", {disptype::commontype::anode, {}, {}});

    auto sequence = std::views::iota(0, (int32_t)servosnum);
    std::ranges::for_each(sequence, [this](uint8_t num) {
        using namespace servo::rpi::pca9685;
        // use spec to provide parameters for api, i.e. Savox
        // SH-0254MG: refreshrate: 240hz; neutral position: 1500us;
        // max travel 800us -> 2200us [150dgr];
        static const auto refreshratehz{240};
        static const auto neutralpos{1500us}, leftpos{800us}, rightpos{2200us};
        auto iface = servo::Factory::create<servo::rpi::pca9685::Servo,
                                            servo::rpi::pca9685::config_t>(
            {"/sys/class/pwm/pwmchip2/", num,
             num % 2 ? mounttype::inverted : mounttype::normal, refreshratehz,
             neutralpos, leftpos, rightpos, this->logif});
        this->servos.push_back(iface);
    });
    auto servosgroup =
        servo::Factory::create<servo::group::Servo, servo::group::config_t>(
            {servos, {}});
    servosgroup->moveto(posoutside);

    ledrgb = ledrgb::Factory::create<ledrgb::rpi5::ws281x::Ledrgb,
                                     ledrgb::rpi5::ws281x::config_t,
                                     ledrgb::rpi5::ws281x::param_t>(
        {servosnum, 255});
    ledrgb::grouptype ledsgroup;
    std::ranges::for_each(sequence, [&ledsgroup](uint8_t num) {
        ledsgroup.emplace_back(num, ledrgb::colortype::red);
    });
    ledrgb->light(ledsgroup);

    graph = graphs::Factory::create<graphs::dygraph::rangesamples::Graph,
                                    graphs::dygraph::rangesamples::configall_t>(
        {{"Servos switching", "time [sec]", "servo nuim [0, 5]"},
         {1200, 400},
         {100ms, 100, {{"data.csv", "time,state", 1000}}}});
    timemonitor = std::make_shared<graphs::helpers::TimeMonitor>();
    // graph->start();
}

StateManager::~StateManager()
{
    graph->stop();
    adc->unobserve(adcobserver);
    auto servosgroup =
        servo::Factory::create<servo::group::Servo, servo::group::config_t>(
            {servos, logif});
    servosgroup->movecenter();
}

bool StateManager::set(uint32_t pos)
{
    static uint32_t statesettimems{};
    if (issetready(pos, statesettimems))
    {
        state.reset();
        auto servo = !pos              ? nullptr
                     : pos > servosnum ? nullptr
                                       : servos.at(pos - 1);
        state = std::make_unique<State>(pos, display, ledrgb, servo, graph,
                                        timemonitor, logif);
        statesettimems = getcurrtimems();
        return true;
    }
    return false;
}

bool StateManager::testrun() const
{
    log(logif, logs::level::info, "Test run procedure started");
    auto servosgroup =
        servo::Factory::create<servo::group::Servo, servo::group::config_t>(
            {servos, logif});
    log(logif, logs::level::info, "Moving servos left");
    display->show(str(1), disptype::colortype::first);
    servosgroup->moveleft();
    getchar();
    log(logif, logs::level::info, "Moving servos right");
    display->show(str(2), disptype::colortype::first);
    servosgroup->moveright();
    getchar();
    log(logif, logs::level::info, "Moving servos middle");
    display->show(str(3), disptype::colortype::first);
    servosgroup->movecenter();
    getchar();
    log(logif, logs::level::info, "Moving servos inside");
    display->show(str(4), disptype::colortype::first);
    servosgroup->moveto(posinside);
    getchar();
    log(logif, logs::level::info, "Moving servos outside");
    servosgroup->moveto(posoutside);
    display->show(str(5), disptype::colortype::first);
    log(logif, logs::level::info, "Test run procedure completed");
    getchar();
    display->show(" ", disptype::colortype::first);
    return true;
}

bool StateManager::issetready(uint32_t pos, uint32_t lastsettimems) const
{
    auto timediff{getcurrtimems() - lastsettimems};
    return pos >= posmin && pos <= posmax && timediff >= interval.count();
}

uint32_t StateManager::getcurrtimems() const
{
    return timemonitor->getmiliseconds();
}
