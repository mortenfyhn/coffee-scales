#include "SmoothingFilter.h"

SmoothingFilter::SmoothingFilter(const unsigned size)
: filter(size), hysteresis(HYSTERESIS_SIZE) {}

void SmoothingFilter::addValue(float value)
{
  filter.addValue(value);
  last_value = value;
}

float SmoothingFilter::getValue()
{
  float value = hasSteadyState() ? getSmoothedValue() : getLastValue();
  return hysteresis.compute(value);
}

bool SmoothingFilter::hasSteadyState()
{
  static const float standard_deviation_threshold = 0.1;
  return filter.getStandardDeviation() < standard_deviation_threshold;
}

float SmoothingFilter::getSmoothedValue()
{
  return filter.getAverage();
}

float SmoothingFilter::getLastValue()
{
  return last_value;
}
