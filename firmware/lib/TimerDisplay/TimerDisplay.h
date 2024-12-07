#pragma once

#include <Display.h>

class TimerDisplay : public Display
{
  public:
    TimerDisplay(uint8_t pin_clk, uint8_t pin_dio) : Display{pin_clk, pin_dio}
    {
    }

    void start();
    void stop();
    void update();

  private:
    unsigned long start_millis = 0;
    bool started = false;
    static constexpr unsigned long millis_per_second = 1000;
    static constexpr unsigned long millis_per_minute = 60 * millis_per_second;
    static constexpr unsigned long millis_per_hour = 60 * millis_per_minute;

    static uint8_t millisToSeconds(unsigned long millis);
    static uint8_t millisToMinutes(unsigned long millis);
    void showTime(uint8_t minutes, uint8_t seconds);
};
