#pragma once

#include <RunningAverage.h>

class SmoothingFilter
{
  public:
    SmoothingFilter(uint8_t filter_size, float steady_threshold)
        : filter_(filter_size), steady_threshold_(steady_threshold)
    {
    }

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
               filter_.getStandardDeviation() < steady_threshold_;
    }

    void clear() { filter_.clear(); }

  private:
    RunningAverage filter_;
    const float steady_threshold_;
    float last_value_;
};
