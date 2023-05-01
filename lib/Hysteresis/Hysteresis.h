#pragma once

class Hysteresis
{
  public:
    Hysteresis(float threshold_size);
    float compute(float value);

  private:
    const float threshold_size = 0.f;
    float upper_threshold = 0.f;
    float lower_threshold = 0.f;
    float curr_value = 0.f;
    void increaseThreshold(float new_upper_threshold);
    void decreaseThreshold(float new_lower_threshold);
};
