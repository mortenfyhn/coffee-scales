#include <Arduino.h>  // millis()
#include <Buffer.h>
#include <TimerDisplay.h>

void TimerDisplay::start()
{
    if (!started)
    {
        start_millis = millis();
        started = true;
        setColonOn(true);
    }
}

void TimerDisplay::stop()
{
    if (started)
    {
        setColonOn(false);
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
    // TODO static buffer?
    auto buf = Buffer::buffer<char>{5};  // 4 digits + null char
    snprintf(buf.data(), 5, "%02i%02i", minutes, seconds);
    print(buf.data());
}
