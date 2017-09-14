#ifndef SMOOTHING_FILTER_H
#define SMOOTHING_FILTER_H

#include <RunningAverage.h>
#include "Hysteresis.h"

class SmoothingFilter
{
public:
  SmoothingFilter(const unsigned size);
  void addValue(float value);
  float getValue();
private:
  static const float HYSTERESIS_SIZE = 0.1;
  RunningAverage filter;
  Hysteresis hysteresis;
  float last_value;
  bool hasSteadyState();
  float getSmoothedValue();
  float getLastValue();
};

#endif
