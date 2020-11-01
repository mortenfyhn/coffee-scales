#ifndef TIMER_DISPLAY_H
#define TIMER_DISPLAY_H

#include <SevenSegmentExtended.h>

class TimerDisplay : public SevenSegmentExtended
{
  public:
    TimerDisplay(uint8_t pin_clk, uint8_t pin_dio);
    void start();
    void stop();
    void update();

  private:
    unsigned long start_millis = millis();
    bool started = false;
    static constexpr unsigned long millis_per_second = 1000;
    static constexpr unsigned long millis_per_minute = 60 * millis_per_second;
    static constexpr unsigned long millis_per_hour = 60 * millis_per_minute;
    static uint8_t millisToSeconds(unsigned long millis);
    static uint8_t millisToMinutes(unsigned long millis);
};

#endif
