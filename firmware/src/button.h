#pragma once

#include "config.h"
#include "serial.h"

class Button
{
  public:
    Button(uint8_t pin) : pin_(pin) {}

    void interrupt_handler()
    {
        // This debounces without waiting.

        debug("isr");

        const uint32_t curr_time_ms = millis();

        // If prev time is zero, then this is the first request, which we should
        // always honour.
        if (prev_time_ms_ == 0ul ||
            curr_time_ms - prev_time_ms_ > config::tare_interval_ms)
        {
            requested_ = true;
            prev_time_ms_ = curr_time_ms;
            debug("long press timer started");
            long_press_start_time_ms = curr_time_ms;
        }
    }

    bool is_pressed()
    {
        if (requested_)
        {
            requested_ = false;
            return true;
        }

        return false;
    }

    bool is_long_pressed()
    {

        // when first pressed, start a timer
        // when we call this function, check if it's still pressed
        // if not, stop the timer and return false
        // if yes

        const bool timer_started = long_press_start_time_ms != 0ul;

        if (!timer_started)
        {
            return false;
        }

        const bool still_pressed = digitalRead(pin_) == LOW;

        if (!still_pressed)
        {
            debug("no longer pressed");
            long_press_start_time_ms = 0ul;
            return false;
        }

        const bool timeout =
            millis() - long_press_start_time_ms > long_press_duration_ms;

        if (!timeout)
        {
            debug("not timed out");
            return false;
        }

        debug("long pressed!");
        long_press_start_time_ms = 0ul;
        return true;
    }

  private:
    uint8_t pin_ = 0;
    bool requested_ = false;
    uint32_t prev_time_ms_ = 0ul;
    uint32_t long_press_start_time_ms = 0ul;
    static constexpr uint32_t long_press_duration_ms = 500;
};
