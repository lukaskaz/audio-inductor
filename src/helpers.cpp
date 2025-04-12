#include "helpers.hpp"

#include "shell/interfaces/linux/bash/shell.hpp"

#include <algorithm>
#include <chrono>

using namespace std::chrono_literals;

void log(std::shared_ptr<logs::LogIf> logif, logs::level level,
         const std::string& msg, const std::source_location loc)
{
    if (logif)
        logif->log(level, std::string{loc.function_name()}, msg);
}

std::string strlower(std::string_view sv)
{
    std::string ret;
    std::ranges::for_each(sv, [&ret](char c) { ret += std::tolower(c); });
    return ret;
}

Async::Async(std::function<void()>&& func)
{
    async = std::async(std::launch::async, func);
}

bool Async::isrunning() const
{
    return async.valid() && async.wait_for(0ms) != std::future_status::ready;
}

Application::Application(const std::string& path, const std::string& name,
                         const std::string& params,
                         std::shared_ptr<logs::LogIf> logif) :
    logif{logif}, name{name}, async{[path, name, params]() {
        shell::Factory::create<shell::lnx::bash::Shell>()->run(
            path + "/" + name + " " + params);
    }}
{
    log(logif, logs::level::info, "Async application started: " + name);
}

bool Application::kill() const
{
    // std::cout << "Killing async app: " << name << std::endl;
    shell::Factory::create<shell::lnx::bash::Shell>()->run("killall -s INT " +
                                                           name);
    log(logif, logs::level::info, "Async application killed: " + name);
    return true;
}
