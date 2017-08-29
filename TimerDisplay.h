#ifndef TIMER_DISPLAY_H
#define TIMER_DISPLAY_H

#include <SevenSegmentExtended.h>

class TimerDisplay
{
public:
  TimerDisplay(uint8_t pin_clk, uint8_t pin_dio);
  void on();
  void off();
  void start();
  void stop();
  void refresh();
private:
  SevenSegmentExtended display;
  unsigned long start_millis;
  bool is_running;
  static const unsigned long millis_per_second = 1000;
  static const unsigned long millis_per_minute = 60 * millis_per_second;
  static const unsigned long millis_per_hour = 60 * millis_per_minute;
  uint8_t millisToSeconds(unsigned long millis);
  uint8_t millisToMinutes(unsigned long millis);
};

#endif
