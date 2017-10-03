#ifndef SMOOTHING_FILTER_H
#define SMOOTHING_FILTER_H

#include <RunningAverage.h>
#include "Hysteresis.h"

class SmoothingFilter
{
public:
  SmoothingFilter(uint8_t filter_size, float hysteresis_size);
  void addValue(float value);
  float getValue();
private:
  static const float STANDARD_DEVIATION_THRESHOLD = 0.1;
  RunningAverage filter;
  Hysteresis hysteresis;
  float last_value;
  bool hasSteadyState();
};

#endif
