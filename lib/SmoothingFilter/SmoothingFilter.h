#pragma once

#include <RunningAverage.h>

class SmoothingFilter
{
  public:
    SmoothingFilter(uint8_t filter_size);
    void addValue(float value);
    float getValue();

  private:
    static constexpr float STANDARD_DEVIATION_THRESHOLD = 0.1;
    RunningAverage filter;
    float last_value;
    bool hasSteadyState() const;
};
