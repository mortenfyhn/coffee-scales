#ifndef SMOOTHING_FILTER_H
#define SMOOTHING_FILTER_H

#include <Hysteresis.h>
#include <RunningAverage.h>

class SmoothingFilter
{
  public:
    SmoothingFilter(uint8_t filter_size, float hysteresis_size);
    void addValue(float value);
    float getValue();

  private:
    static constexpr float STANDARD_DEVIATION_THRESHOLD = 0.1;
    RunningAverage filter;
    Hysteresis hysteresis;
    float last_value;
    bool hasSteadyState() const;
};

#endif
