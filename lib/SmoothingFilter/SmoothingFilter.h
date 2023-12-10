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
        return filter_.getStandardDeviation() < STANDARD_DEVIATION_THRESHOLD;
    }

  private:
    static constexpr float STANDARD_DEVIATION_THRESHOLD = 0.1;
    RunningAverage filter_;
    float last_value_;
};
