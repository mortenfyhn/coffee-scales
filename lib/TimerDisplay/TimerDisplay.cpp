#include "TimerDisplay.h"

TimerDisplay::TimerDisplay(uint8_t pin_clk, uint8_t pin_dio)
    : SevenSegmentExtended(pin_clk, pin_dio)
{
    init();
    setBacklight(100);
}

void TimerDisplay::start()
{
    if (!started)
    {
        start_millis = millis();
        started = true;
    }
}

void TimerDisplay::stop()
{
    clear();
    started = false;
}

void TimerDisplay::update()
{
    if (started)
    {
        const auto elapsed_millis = millis() - start_millis;
        const auto seconds = millisToSeconds(elapsed_millis);
        const auto minutes = millisToMinutes(elapsed_millis);

        printTime(minutes, seconds, false);
    }
}

uint8_t TimerDisplay::millisToSeconds(unsigned long millis)
{
    const auto millis_past_minute = millis % millis_per_minute;
    const auto seconds =
        static_cast<uint8_t>(millis_past_minute / millis_per_second);
    return seconds;
}

uint8_t TimerDisplay::millisToMinutes(unsigned long millis)
{
    const auto millis_past_hour = millis % millis_per_hour;
    const auto minutes =
        static_cast<uint8_t>(millis_past_hour / millis_per_minute);
    return minutes;
}
