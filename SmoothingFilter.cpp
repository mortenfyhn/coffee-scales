#include "SmoothingFilter.h"

SmoothingFilter::SmoothingFilter()
: filter(FILTER_SIZE), hysteresis(HYSTERESIS_SIZE) {}

void SmoothingFilter::addValue(float value)
{
  filter.addValue(value);
  last_value = value;
}

float SmoothingFilter::getValue()
{
  float value = hasSteadyState() ? filter.getAverage() : last_value;
  return hysteresis.compute(value);
}

bool SmoothingFilter::hasSteadyState()
{
  return filter.getStandardDeviation() < STANDARD_DEVIATION_THRESHOLD;
}
