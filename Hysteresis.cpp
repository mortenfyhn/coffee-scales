#include "Hysteresis.h"

Hysteresis::Hysteresis(float threshold_size)
: threshold_size(threshold_size) {}

float Hysteresis::compute(float value)
{
  if (value > upper_threshold)
    increaseThreshold(value);
  else if (value < lower_threshold)
    decreaseThreshold(value);
  return curr_value;
}

void Hysteresis::increaseThreshold(float new_upper_threshold)
{
  upper_threshold = new_upper_threshold;
  lower_threshold = upper_threshold - threshold_size;
  curr_value = new_upper_threshold;
}

void Hysteresis::decreaseThreshold(float new_lower_threshold)
{
  lower_threshold = new_lower_threshold;
  upper_threshold = lower_threshold + threshold_size;
  curr_value = new_lower_threshold;
}
