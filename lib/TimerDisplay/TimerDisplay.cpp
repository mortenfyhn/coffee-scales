#include "TimerDisplay.h"

TimerDisplay::TimerDisplay(uint8_t pin_clk, uint8_t pin_dio)
    : SevenSegmentExtended(pin_clk, pin_dio)
{
    init();
    setBacklight(100);
    start_millis = millis();
    is_running = false;
}

void TimerDisplay::start()
{
    if (!is_running)
    {
        start_millis = millis();
        is_running = true;
    }
}

void TimerDisplay::stop()
{
    clear();
    is_running = false;
}

void TimerDisplay::refresh()
{
    if (is_running)
    {
        unsigned long elapsed_millis = millis() - start_millis;
        uint8_t seconds = millisToSeconds(elapsed_millis);
        uint8_t minutes = millisToMinutes(elapsed_millis);

        printTime(minutes, seconds, false);
    }
}

uint8_t TimerDisplay::millisToSeconds(unsigned long millis)
{
    unsigned long millis_past_minute = (millis % millis_per_minute);
    uint8_t seconds = (uint8_t)(millis_past_minute / millis_per_second);
    return seconds;
}

uint8_t TimerDisplay::millisToMinutes(unsigned long millis)
{
    unsigned long millis_past_hour = (millis % millis_per_hour);
    uint8_t minutes = (uint8_t)(millis_past_hour / millis_per_minute);
    return minutes;
}
