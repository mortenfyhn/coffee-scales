#ifndef SMOOTHING_FILTER_H
#define SMOOTHING_FILTER_H

#include <RunningAverage.h>

class SmoothingFilter
{
public:
  SmoothingFilter(const unsigned size);
  void addValue(float value);
  float getValue();
private:
  RunningAverage filter;
  float last_value;
  bool hasSteadyState();
  float getSmoothedValue();
  float getLastValue();
};

#endif
