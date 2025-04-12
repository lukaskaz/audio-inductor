#include "adc/interfaces/rpi/ads1115/adc.hpp"
#include "display/interfaces/sevseg/m74hc595/single/bicolor/display.hpp"
#include "gpio/interfaces/rpi/native/gpio.hpp"
#include "graphs/interfaces/dygraph/rangesamples/graph.hpp"
#include "ledrgb/interfaces/rpi5/ws281x/ledrgb.hpp"
#include "logs/interfaces/console/logs.hpp"
#include "logs/interfaces/group/logs.hpp"
#include "logs/interfaces/storage/logs.hpp"
#include "pwm/interfaces/rpi/pca9685/pwm.hpp"
#include "servo/interfaces/digital/rpi/pca9685/servo.hpp"
#include "servo/interfaces/group/servo.hpp"
#include "shell/interfaces/linux/bash/shell.hpp"
// #include "stt/interfaces/v2/googleapi.hpp"
#include "tts/interfaces/googlecloud.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <ranges>
#include <source_location>
#include <string>
#include <vector>

using namespace std::string_literals;
namespace disptype = display::sevseg::m74hc595::single::bicolor;
namespace ledrgbtype = ledrgb::rpi5::ws281x;
using DisplayIf = display::DisplayIf<disptype::param_t>;
using LedrgdIf = ledrgb::LedrgbIf<ledrgbtype::param_t>;

void log(std::shared_ptr<logs::LogIf>, logs::level, const std::string&,
         const std::source_location loc = std::source_location::current());

std::string str(const auto& value)
{
    if constexpr (std::is_same<const std::string&, decltype(value)>())
        return value;
    else
        return std::to_string(value);
}

class StateIf
{
  public:
    virtual ~StateIf() = default;
    virtual void run() = 0;
};

class State : public StateIf
{
  public:
    explicit State(uint32_t pos, std::shared_ptr<DisplayIf> display,
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

    ~State()
    {
        if (servo)
            servo->moveto(10.);
        if (pos)
            ledrgb->light(pos - 1, ledrgb::colortype::red);
        auto timems{str(tm->getmiliseconds())};
        graph->add(timems + separator + str(pos));
        log(logif, logs::level::debug, "Removed state @ pos: " + str(pos));
    }

    void run() override
    {
        if (servo)
            servo->moveto(-25.);
        if (pos)
            ledrgb->light(pos - 1, ledrgb::colortype::green);
        display->show(str(pos), !pos ? disptype::colortype::second
                                     : disptype::colortype::first);
    }

  private:
    const std::string separator{","};
    const std::string timems{","};
    const uint32_t pos;
    const std::shared_ptr<logs::LogIf> logif;
    const std::shared_ptr<DisplayIf> display;
    const std::shared_ptr<LedrgdIf> ledrgb;
    const std::shared_ptr<servo::ServoIf> servo;
    const std::shared_ptr<graphs::GraphIf> graph;
    std::shared_ptr<graphs::helpers::TimeMonitor> tm;
};

class StateHandler
{
  public:
    StateHandler(std::shared_ptr<logs::LogIf> logif) : logif{logif}
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

        display =
            display::Factory::create<disptype::Display, disptype::config_t,
                                     disptype::param_t>(
                "/dev/spidev0.0", {disptype::commontype::anode, {}, {}});

        auto sequence = std::views::iota(0, (int32_t)servosnum);
        std::ranges::for_each(sequence, [this](uint8_t num) {
            using namespace servo::rpi::pca9685;
            // use spec to provide parameters for api, i.e. Savox
            // SH-0254MG: refreshrate: 240hz; neutral position: 1500us;
            // max travel 800us -> 2200us [150dgr];
            static const auto refreshratehz{240};
            static const auto neutralpos{1500us}, leftpos{800us},
                rightpos{2200us};
            auto iface = servo::Factory::create<servo::rpi::pca9685::Servo,
                                                servo::rpi::pca9685::config_t>(
                {"/sys/class/pwm/pwmchip2/",
                 num,
                 num % 2 ? mounttype::inverted : mounttype::normal,
                 refreshratehz,
                 neutralpos,
                 leftpos,
                 rightpos,
                 {}});
            this->servos.push_back(iface);
        });
        auto servosgroup =
            servo::Factory::create<servo::group::Servo, servo::group::config_t>(
                {servos, {}});
        servosgroup->moveto(10.);

        ledrgb = ledrgb::Factory::create<ledrgb::rpi5::ws281x::Ledrgb,
                                         ledrgb::rpi5::ws281x::config_t,
                                         ledrgb::rpi5::ws281x::param_t>(
            {servosnum, 255});
        ledrgb::grouptype ledsgroup;
        std::ranges::for_each(sequence, [&ledsgroup](uint8_t num) {
            ledsgroup.emplace_back(num, ledrgb::colortype::red);
        });
        ledrgb->light(ledsgroup);

        graph =
            graphs::Factory::create<graphs::dygraph::rangesamples::Graph,
                                    graphs::dygraph::rangesamples::configall_t>(
                {{"Servos switching", "time [sec]", "servo nuim [0, 5]"},
                 {1200, 400},
                 {100ms, 100, {{"data.csv", "time,state", 1000}}}});
        timemonitor = std::make_shared<graphs::helpers::TimeMonitor>();
        // graph->start();
    }

    ~StateHandler()
    {
        graph->stop();
        adc->unobserve(adcobserver);
        auto servosgroup =
            servo::Factory::create<servo::group::Servo, servo::group::config_t>(
                {servos, logif});
        servosgroup->movecenter();
    }

    bool set(uint32_t pos)
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

    bool testrun() const
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
        log(logif, logs::level::info, "Moving servos front");
        display->show(str(4), disptype::colortype::first);
        servosgroup->moveto(-25.);
        getchar();
        log(logif, logs::level::info, "Moving servos back");
        servosgroup->moveto(10.);
        display->show(str(5), disptype::colortype::first);
        log(logif, logs::level::info, "Test run procedure completed");
        getchar();
        display->show(" ", disptype::colortype::first);
        return true;
    }

  private:
    const uint32_t posmin{0}, posmax{4}, servosnum{4};
    const std::chrono::milliseconds interval{80ms};
    const std::shared_ptr<logs::LogIf> logif;
    std::unique_ptr<StateIf> state;
    std::shared_ptr<adc::AdcIf> adc;
    std::shared_ptr<pwm::PwmIf> pwm;
    std::vector<std::shared_ptr<servo::ServoIf>> servos;
    std::shared_ptr<DisplayIf> display;
    std::shared_ptr<LedrgdIf> ledrgb;
    std::shared_ptr<graphs::GraphIf> graph;
    std::shared_ptr<graphs::helpers::TimeMonitor> timemonitor;
    std::shared_ptr<helpers::Observer<adc::ObsData>> adcobserver;

    bool issetready(uint32_t pos, uint32_t lastsettimems) const
    {
        auto timediff{getcurrtimems() - lastsettimems};
        return pos >= posmin && pos <= posmax && timediff >= interval.count();
    }

    uint32_t getcurrtimems() const
    {
        return timemonitor->getmiliseconds();
    }
};

class Async
{
  public:
    explicit Async(std::function<void()>&& func)
    {
        async = std::async(std::launch::async, func);
    }

    bool isrunning() const
    {
        return async.wait_for(0ms) != std::future_status::ready;
    }

  private:
    std::future<void> async;
};

class Application
{
  public:
    Application(const std::string& path, const std::string& name,
                const std::string& params) :
        name{name}, async{[path, name, params]() {
            shell::Factory::create<shell::lnx::bash::Shell>()->run(
                path + "/" + name + " " + params);
        }}
    {
        std::cout << "Starting async app: " << name << std::endl;
    }

    bool kill()
    {
        std::cout << "Killing async app: " << name << std::endl;
        shell::Factory::create<shell::lnx::bash::Shell>()->run(
            "killall -s INT " + name);
        return true;
    }

  private:
    const std::string name;
    Async async;
};

void log(std::shared_ptr<logs::LogIf> logif, logs::level level,
         const std::string& msg, const std::source_location loc)
{
    if (logif)
        logif->log(level, std::string{loc.function_name()}, msg);
}

std::string strlower(std::string_view str)
{
    std::string ret;
    std::ranges::for_each(str, [&ret](char c) { ret += std::tolower(c); });
    return ret;
}

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        static const std::string cavafifo{"/tmp/cavaout"};
        Application app{"./cava-project/build/bin", "cava",
                        "-p ../conf/cava.conf"};
        // auto graph = graphs::GraphFactory::create<graphs::dygraph::Graph>(
        //     {"Servos switching", "time", "servo num"}, {500, 300},
        //     {100ms, 100, {{"data.csv", "time,state"}}});
        // $ amixer -c 8 sset PCM 5%

        // uint32_t pin{14};
        while (!std::filesystem::exists(cavafifo))
            usleep(100);
        std::ifstream ifs(cavafifo);
        if (!ifs.is_open())
            throw std::runtime_error("Cannot open pipe " + cavafifo);
        // [[maybe_unused]] auto clearfifo =
        //     std::string(std::istreambuf_iterator<char>(ifs.rdbuf()), {});

        auto loglvl =
            (bool)atoi(argv[1]) ? logs::level::debug : logs::level::info;

        auto logconsole =
            logs::Factory::create<logs::console::Log, logs::console::config_t>(
                {loglvl, logs::time::hide, logs::tags::hide});
        auto logstorage =
            logs::Factory::create<logs::storage::Log, logs::storage::config_t>(
                {loglvl, logs::time::show, logs::tags::show, {}});
        auto logif =
            logs::Factory::create<logs::group::Log, logs::group::config_t>(
                {logconsole, logstorage});

        auto swpin = gpio::Factory::create<gpio::rpi::native::Gpio,
                                           gpio::rpi::native::config_t>(
            {gpio::rpi::native::modetype::input, {21}, logif});

        auto handler{StateHandler{logif}};
        uint32_t mode{};
        auto swobs = Observer<gpio::GpioData>::create(
            [&mode](const gpio::GpioData& data) { mode = std::get<1>(data); });
        swpin->observe(21, swobs);

        auto buzzpin = gpio::Factory::create<gpio::rpi::native::Gpio,
                                             gpio::rpi::native::config_t>(
            {gpio::rpi::native::modetype::output_normal, {17}, logif});

        auto speech =
            tts::TextToVoiceFactory::create<tts::googlecloud::TextToVoice,
                                            tts::googlecloud::configmin_t>(
                {{tts::language::polish, tts::gender::female, 1}, logif});

        // auto stt =
        //     stt::TextFromVoiceFactory::create<stt::v2::googleapi::TextFromVoice,
        //                                       stt::v2::googleapi::configmin_t>(
        //         {stt::language::polish, {}, logif});

        while (mode != 1)
        {
            if (mode == 2)
            {
                uint32_t vulgarity{90};
                speech->speak("Uwaga, rozpoczynam procedurę testową, pasuje?");
                auto response = speech->listen();
                while (strlower(response.first).find("tak") != 0)
                {
                    log(logif, logs::level::info,
                        "Response: " + response.first);
                    if (strlower(response.first).find("zmniejsz wulgarność") !=
                        std::string::npos)
                    {
                        speech->speak("O ile?");
                        response = speech->listen();
                        vulgarity -= atoi(response.first.c_str());
                        speech->speak("Teraz mam " + str(vulgarity) +
                                      "%, będę grzeczna kurde");
                        response = speech->listen();
                        if (strlower(response.first).find("chcesz mniej") !=
                            std::string::npos)
                            speech->speak("Wyluzuj, jest git!");
                    }
                    else if (strlower(response.first).find("wulgarność") !=
                             std::string::npos)
                    {
                        speech->speak("Ustawiona na " + str(vulgarity) + "%");
                    }
                    else if (strlower(response.first).find("zakończ program") !=
                             std::string::npos)
                    {
                        speech->speak(
                            "Buuu nie chcem, ale jak trzeba to zamykam się :(");
                        app.kill();
                        return 2;
                    }
                    else
                        speech->speak("Noż kurwa, nie rozumiem!");
                    response = speech->listen();
                }
                handler.testrun();
                speech->speak("No i zakończyłam procedurę testową, hura!");
                buzzpin->write(17, 1);
                sleep(1);
                buzzpin->write(17, 0);
                mode = 0;
            }
            if (mode == 5)
            {
                app.kill();
                return 1;
            }
            else
                usleep(100 * 1000);
        }
        // swpin->unobserve(21, swobs);

        auto exitmonitor = Async([]() { getchar(); });
        speech->speak("Startuję z interpreterem muzyki");
        log(logif, logs::level::info, "Audio inductor is started");
        while (ifs.good() && exitmonitor.isrunning())
        {
            static auto prev{(char)0xFF};
            if (auto curr{(char)ifs.get()}; std::isdigit(curr) && curr != prev)
            {
                if (handler.set(atoi(&curr)))
                {
                    log(logif, logs::level::info,
                        "Audio level: " + str(atoi(&curr)));
                    prev = curr;
                }
            }
        }
        app.kill();
        speech->speak("Przerwałeś analizę dzwięku, a tak ładnie grało kurczę");
    }
    return 0;
}
