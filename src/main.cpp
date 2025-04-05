#include "display/interfaces/sevseg/m74hc595/single/bicolor/display.hpp"
#include "graphs/interfaces/dygraph/rangesamples/graph.hpp"
#include "logs/interfaces/console/logs.hpp"
#include "logs/interfaces/group/logs.hpp"
#include "logs/interfaces/storage/logs.hpp"
#include "shell/interfaces/linux/bash/shell.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>

using namespace std::string_literals;
namespace disptype = display::sevseg::m74hc595::single::bicolor;
using DisplayIf = display::DisplayIf<disptype::param_t>;

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
                   std::shared_ptr<logs::LogIf> logif) :
        logif{logif},
        pos{pos}, display{display}
    {
        log(logif, logs::level::debug, "Created state @ pos: " + str(pos));
    }

    ~State()
    {
        log(logif, logs::level::debug, "Removed state @ pos: " + str(pos));
    }

    void run() override
    {
        display->show(str(pos), !pos ? disptype::colortype::second
                                     : disptype::colortype::first);
    }

  private:
    std::shared_ptr<logs::LogIf> logif;
    uint32_t pos;
    std::shared_ptr<DisplayIf> display;
};

class StateHandler
{
  public:
    StateHandler(std::shared_ptr<logs::LogIf> logif) : logif{logif}
    {
        display =
            display::Factory::create<disptype::Display, disptype::config_t,
                                     disptype::param_t>(
                "/dev/spidev0.0", {disptype::commontype::anode, {}, {}});
    }

    void set(uint32_t pos)
    {
        state = std::make_unique<State>(pos, display, logif);
        state->run();
    }

  private:
    const std::shared_ptr<logs::LogIf> logif;
    std::unique_ptr<StateIf> state;
    std::shared_ptr<DisplayIf> display;
};

class Async
{
  public:
    explicit Async(std::function<void()>&& func)
    {
        async = std::async(std::launch::async, func);
    }

  private:
    std::future<void> async;
};

class Application
{
  public:
    Application(const std::string& path, const std::string& name,
                const std::string& params) :
        name{name},
        async{[path, name, params]() {
            shell::Factory::create<shell::lnx::bash::Shell>()->run(
                path + "/" + name + " " + params);
        }}
    {
        std::cout << "Starting async app: " << name << std::endl;
    }

    ~Application()
    {
        std::cout << "Killing async app: " << name << std::endl;
        shell::Factory::create<shell::lnx::bash::Shell>()->run(
            "killall -s KILL " + name);
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

        auto graph =
            graphs::Factory::create<graphs::dygraph::rangesamples::Graph,
                                    graphs::dygraph::rangesamples::configall_t>(
                {{"Servos switching", "time [sec]", "servo nuim [0, 5]"},
                 {1200, 400},
                 {100ms, 100, {{"data.csv", "time,state", 1000}}}});

        // uint32_t pin{14};
        while (!std::filesystem::exists(cavafifo))
            usleep(100);
        std::ifstream ifs(cavafifo);
        if (!ifs.is_open())
        {
            throw std::runtime_error("Cannot open pipe " + cavafifo);
        }
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

        auto handler{StateHandler{logif}};
        graph->start();
        const auto separator{","s};
        auto tm{graphs::helpers::TimeMonitor()};

        log(logif, logs::level::info, "Audio inductor is started");
        while (ifs.good())
        {
            static auto prev{(char)0xFF};
            if (auto curr{(char)ifs.get()}; std::isdigit(curr) && curr != prev)
            {
                auto currval{atoi(&curr)}, prevval{atoi(&prev)};
                handler.set(currval);
                auto timems = str(tm.getmiliseconds());
                graph->add(timems + separator + str(prevval));
                graph->add(timems + separator + str(currval));
                log(logif, logs::level::debug, "Audio level: " + str(currval));
                prev = curr;
            }
        }
    }
    return 0;
}
