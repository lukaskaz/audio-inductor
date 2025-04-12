#include "helpers.hpp"
#include "speech/stt/interfaces/v1/googlecloud.hpp"
#include "speech/tts/interfaces/googlecloud.hpp"
#include "speechtexts.hpp"
#include "statemanager.hpp"

#include <filesystem>
#include <fstream>

namespace ttstype = tts::googlecloud;
namespace stttype = stt::v1::googlecloud;

static const auto speechlanguage = tts::language::polish;

int main(int argc, char** argv)
{
    if (argc > 1)
    {
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

        auto tts = tts::TextToVoiceFactory::create<ttstype::TextToVoice,
                                                   ttstype::configmin_t>(
            {{speechlanguage, tts::gender::female, 1}, logif});
        auto stt = stt::TextFromVoiceFactory::create<stttype::TextFromVoice,
                                                     stttype::configmin_t>(
            {stt::language::polish, {}, logif});

        tts->speak(getspeechtext(task::initiatating, speechlanguage));

        static const std::string cavafifo{"/tmp/cavaout"};
        Application app{"./cava-project/build/bin", "cava",
                        "-p ../conf/cava.conf", logif};
        // auto graph = graphs::GraphFactory::create<graphs::dygraph::Graph>(
        //     {"Servos switching", "time", "servo num"}, {500, 300},
        //     {100ms, 100, {{"data.csv", "time,state"}}});
        // $ amixer -c 8 sset PCM 5%

        while (!std::filesystem::exists(cavafifo))
            usleep(100);
        std::ifstream ifs(cavafifo);
        if (!ifs.is_open())
            throw std::runtime_error("Cannot open pipe " + cavafifo);
        // [[maybe_unused]] auto clearfifo =
        //     std::string(std::istreambuf_iterator<char>(ifs.rdbuf()), {});

        auto state{StateManager{logif}};
        const uint32_t switchpin{21}, buzzerpin{17};
        enum class modetype
        {
            idle,
            analyze,
            testrun,
            exit
        } mode;

        auto swgpio = gpio::Factory::create<gpio::rpi::native::Gpio,
                                            gpio::rpi::native::config_t>(
            {gpio::rpi::native::modetype::input, {switchpin}, logif});
        auto swobserver = gpio::helpers::Observer<gpio::GpioData>::create(
            [&mode](const gpio::GpioData& data) {
                mode = (modetype)std::get<1>(data);
            });
        swgpio->observe(switchpin, swobserver);

        auto buzzgpio = gpio::Factory::create<gpio::rpi::native::Gpio,
                                              gpio::rpi::native::config_t>(
            {gpio::rpi::native::modetype::output_normal, {buzzerpin}, logif});

        tts->speak(getspeechtext(task::ready, speechlanguage));
        tts->speak(getspeechtext(task::selectmode, speechlanguage));
        while (true)
        {
            if (mode == modetype::analyze)
            {
                break;
            }
            else if (mode == modetype::testrun)
            {
                uint32_t vulgarity{90};
                tts->speak(getspeechtext(task::testrunstart, speechlanguage));
                auto response = stt->listen();
                while (strlower(response.first).find("tak") != 0)
                {
                    log(logif, logs::level::info,
                        "Response: " + response.first);
                    if (strlower(response.first).find("zmniejsz wulgarność") !=
                        std::string::npos)
                    {
                        tts->speak("O ile?");
                        response = stt->listen();
                        vulgarity -= atoi(response.first.c_str());
                        tts->speak("Teraz mam " + str(vulgarity) +
                                   "%, będę grzeczna kurde");
                        response = stt->listen();
                        if (strlower(response.first).find("chcesz mniej") !=
                            std::string::npos)
                            tts->speak("Wyluzuj, jest git!");
                    }
                    else if (strlower(response.first).find("wulgarność") !=
                             std::string::npos)
                    {
                        tts->speak("Ustawiona na " + str(vulgarity) + "%");
                    }
                    else if (strlower(response.first).find("zakończ program") !=
                             std::string::npos)
                    {
                        tts->speak(
                            getspeechtext(task::programexit, speechlanguage));
                        app.kill();
                        return 2;
                    }
                    else
                        tts->speak("Noż kurwa, nie rozumiem!");
                    response = stt->listen();
                }
                state.testrun();
                tts->speak(getspeechtext(task::testrunend, speechlanguage));
                buzzgpio->write(17, 1);
                sleep(1);
                buzzgpio->write(17, 0);
                mode = modetype::idle;
            }
            if (mode == modetype::exit)
            {
                app.kill();
                return 1;
            }
            else
                usleep(100 * 1000);
        }
        swgpio->unobserve(switchpin, swobserver);

        auto exitmonitor = Async([]() { getchar(); });
        tts->speak(getspeechtext(task::analyzestart, speechlanguage));
        log(logif, logs::level::info, "Audio inductor is started");
        while (ifs.good() && exitmonitor.isrunning())
        {
            static auto prev{(char)0xFF};
            if (auto curr{(char)ifs.get()}; std::isdigit(curr) && curr != prev)
            {
                if (state.set(atoi(&curr)))
                {
                    log(logif, logs::level::info,
                        "Audio level: " + str(atoi(&curr)));
                    prev = curr;
                }
            }
        }
        tts->speakasync(getspeechtext(task::analyzeend, speechlanguage));
        app.kill();
    }
    return 0;
}
