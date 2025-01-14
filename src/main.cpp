#include "graphs/interfaces/dygraph.hpp"
#include "shellcommand.hpp"

#include <wiringPi.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>

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
        name{name}, async{[path, name, params]() {
            std::make_shared<shell::BashCommand>()->run(path + "/" + name +
                                                        " " + params);
        }}
    {
        std::cout << "Starting async app: " << name << std::endl;
    }

    ~Application()
    {
        std::cout << "Killing async app: " << name << std::endl;
        shell::BashCommand().run("killall -s KILL " + name);
    }

  private:
    const std::string name;
    Async async;
};

int main()
{
    static const std::string cavafifo{"/tmp/cavaout"};
    Application app{"./cava-project/build/bin", "cava", "-p ../conf/cava.conf"};
    auto graph = graphs::GraphFactory::create<graphs::dygraph::Graph>(
        {"Servos switching", "time", "servo num"}, {500, 300},
        {100ms, 100, {{"data.csv", "time,state"}}});

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
    while (ifs.good())
    {
        static auto prev{(char)0xFF};
        if (auto curr{(char)ifs.get()}; std::isdigit(curr) && curr != prev)
        {
            auto timestamp{graphs::gettimestamp()};
            graph->add(timestamp + "," + std::to_string(atoi(&prev)));
            graph->add(timestamp + "," + std::to_string(atoi(&curr)));
            digitalWrite(pin, !digitalRead(pin));
            prev = curr;
        }
    }

    return 0;
}
