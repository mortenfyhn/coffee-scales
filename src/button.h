#pragma once

#include "config.h"

class Button
{
  public:
    void pressed()
    {
        // This debounces without waiting.

        const auto curr_time_ms = millis();

        // If prev time is zero, then this is the first request, which we should
        // always honour.
        if (prev_time_ms_ == 0ul ||
            curr_time_ms - prev_time_ms_ > config::tare_interval_ms)
        {
            requested_ = true;
            prev_time_ms_ = curr_time_ms;
        }
    }

    bool should_tare()
    {
        if (requested_)
        {
            requested_ = false;
            return true;
        }

        return false;
    }

  private:
    bool requested_ = false;
    unsigned long prev_time_ms_ = 0ul;
};