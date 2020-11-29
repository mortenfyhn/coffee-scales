#ifndef TIMER_DISPLAY_H
#define TIMER_DISPLAY_H

#include <TM1637Display.h>

class TimerDisplay
{
  public:
    TimerDisplay(uint8_t pin_clk, uint8_t pin_dio, uint8_t brightness);
    void start();
    void stop();
    void update();

  private:
    TM1637Display display;
    unsigned long start_millis = 0;
    bool started = false;
    static constexpr unsigned long millis_per_second = 1000;
    static constexpr unsigned long millis_per_minute = 60 * millis_per_second;
    static constexpr unsigned long millis_per_hour = 60 * millis_per_minute;
    static uint8_t millisToSeconds(unsigned long millis);
    static uint8_t millisToMinutes(unsigned long millis);
};

#endif
