#pragma once

#include "logs/interfaces/logs.hpp"

#include <functional>
#include <future>
#include <memory>
#include <source_location>
#include <string>

std::string str(const auto& value)
{
    if constexpr (std::is_same<const std::string&, decltype(value)>())
        return value;
    else
        return std::to_string(value);
}

void log(std::shared_ptr<logs::LogIf>, logs::level, const std::string&,
         const std::source_location loc = std::source_location::current());

std::string strlower(std::string_view);

class Async
{
  public:
    explicit Async(std::function<void()>&&);
    bool isrunning() const;

  private:
    std::future<void> async;
};

class Application
{
  public:
    Application(const std::string&, const std::string&, const std::string&,
                std::shared_ptr<logs::LogIf>);
    bool kill() const;

  private:
    const std::shared_ptr<logs::LogIf> logif;
    const std::string name;
    Async async;
};