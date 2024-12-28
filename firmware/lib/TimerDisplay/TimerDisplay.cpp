#include <Arduino.h>  // millis()
#include <TimerDisplay.h>

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
    if (started)
    {
        clear();
        started = false;
    }
}

void TimerDisplay::update()
{
    if (started)
    {
        const uint32_t elapsed_millis = millis() - start_millis;
        const uint8_t seconds = millisToSeconds(elapsed_millis);
        const uint8_t minutes = millisToMinutes(elapsed_millis);

        showTime(minutes, seconds);
    }
}

uint8_t TimerDisplay::millisToSeconds(uint32_t millis)
{
    const uint32_t millis_past_minute = millis % millis_per_minute;
    const auto seconds =
        static_cast<uint8_t>(millis_past_minute / millis_per_second);
    return seconds;
}

uint8_t TimerDisplay::millisToMinutes(unsigned long millis)
{
    const uint32_t millis_past_hour = millis % millis_per_hour;
    const auto minutes =
        static_cast<uint8_t>(millis_past_hour / millis_per_minute);
    return minutes;
}

void TimerDisplay::showTime(uint8_t minutes, uint8_t seconds)
{
    static constexpr uint8_t colon = 0b01000000;
    showNumberDecEx(minutes, colon, true, 2, 0);
    showNumberDecEx(seconds, 0, true, 2, 2);
}
