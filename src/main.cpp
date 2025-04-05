#include "graphs/interfaces/dygraph/rangesamples/graph.hpp"
#include "shell/interfaces/linux/bash/shell.hpp"

#include <wiringPi.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>

using namespace std::string_literals;

std::string str(const auto& value)
{
    if constexpr (std::is_same<const std::string&, decltype(value)>())
        return value;
    else
        return std::to_string(value);
}

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

int main()
{
    static const std::string cavafifo{"/tmp/cavaout"};
    Application app{"./cava-project/build/bin", "cava", "-p ../conf/cava.conf"};
    // auto graph = graphs::GraphFactory::create<graphs::dygraph::Graph>(
    //     {"Servos switching", "time", "servo num"}, {500, 300},
    //     {100ms, 100, {{"data.csv", "time,state"}}});

    auto graph =
        graphs::Factory::create<graphs::dygraph::rangesamples::Graph,
                                graphs::dygraph::rangesamples::configall_t>(
            {{"Servos switching", "time [sec]", "servo nuim [0, 5]"},
             {1200, 400},
             {100ms, 100, {{"data.csv", "time,state", 1000}}}});

    uint32_t pin{14};
    wiringPiSetupGpio();
    pinMode(pin, OUTPUT);
    while (!std::filesystem::exists(cavafifo))
    {
        usleep(100);
    }
    std::ifstream ifs(cavafifo);
    if (!ifs.is_open())
    {
        throw std::runtime_error("Cannot open pipe " + cavafifo);
    }
    // [[maybe_unused]] auto clearfifo =
    //     std::string(std::istreambuf_iterator<char>(ifs.rdbuf()), {});

    graph->start();
    const auto separator{","s};
    auto tm{graphs::helpers::TimeMonitor()};
    while (ifs.good())
    {
        static auto prev{(char)0xFF};
        if (auto curr{(char)ifs.get()}; std::isdigit(curr) && curr != prev)
        {
            auto timems = str(tm.getmiliseconds());
            graph->add(timems + separator + str(atoi(&prev)));
            graph->add(timems + separator + str(atoi(&curr)));
            digitalWrite(pin, !digitalRead(pin));
            std::cout << "cava val: " << curr << std::endl;
            prev = curr;
        }
    }

    return 0;
}
