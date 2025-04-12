#pragma once

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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace disptype = display::sevseg::m74hc595::single::bicolor;
namespace ledrgbtype = ledrgb::rpi5::ws281x;
using DisplayIf = display::DisplayIf<disptype::param_t>;
using LedrgdIf = ledrgb::LedrgbIf<ledrgbtype::param_t>;

class StateIf
{
  public:
    virtual ~StateIf() = default;
    virtual void run() = 0;
};

class State : public StateIf
{
  public:
    State(uint32_t, std::shared_ptr<DisplayIf>, std::shared_ptr<LedrgdIf>,
          std::shared_ptr<servo::ServoIf>, std::shared_ptr<graphs::GraphIf>,
          std::shared_ptr<graphs::helpers::TimeMonitor>,
          std::shared_ptr<logs::LogIf>);

    ~State();
    void run() override;

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

class StateManager
{
  public:
    explicit StateManager(std::shared_ptr<logs::LogIf>);
    ~StateManager();

    bool set(uint32_t);
    bool testrun() const;

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

    bool issetready(uint32_t, uint32_t) const;
    uint32_t getcurrtimems() const;
};
