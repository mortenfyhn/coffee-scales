#pragma once

#include <RunningAverage.h>

class SmoothingFilter
{
  public:
    SmoothingFilter(uint8_t filter_size) : filter_(filter_size) {}

    void addValue(float value)
    {
        filter_.addValue(value);
        last_value_ = value;
    }

    float getValue()
    {
        return hasSteadyState() ? filter_.getAverage() : last_value_;
    }

    bool hasSteadyState() const
    {
        // If the buffer isn't full, we don't have enough data to determine if
        // it's in steady state, so we require it.
        return filter_.bufferIsFull() &&
               filter_.getStandardDeviation() < STANDARD_DEVIATION_THRESHOLD;
    }

    void clear() { filter_.clear(); }

  private:
    static constexpr float STANDARD_DEVIATION_THRESHOLD = 0.1;
    RunningAverage filter_;
    float last_value_;
};
