#include <SmoothingFilter.h>

SmoothingFilter::SmoothingFilter(uint8_t filter_size) : filter(filter_size) {}

void SmoothingFilter::addValue(float value)
{
    filter.addValue(value);
    last_value = value;
}

float SmoothingFilter::getValue()
{
    return hasSteadyState() ? filter.getAverage() : last_value;
}

bool SmoothingFilter::hasSteadyState() const
{
    return filter.getStandardDeviation() < STANDARD_DEVIATION_THRESHOLD;
}
