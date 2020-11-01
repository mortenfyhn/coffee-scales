#include "SmoothingFilter.h"

SmoothingFilter::SmoothingFilter(uint8_t filter_size, float hysteresis_size)
    : filter(filter_size), hysteresis(hysteresis_size)
{
}

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
