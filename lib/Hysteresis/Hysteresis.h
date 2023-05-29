#pragma once

class Hysteresis
{
  public:
    Hysteresis(float threshold_size) : threshold_size(threshold_size) {}

    float compute(float value)
    {
        if (value > upper_threshold)
            increaseThreshold(value);
        else if (value < lower_threshold)
            decreaseThreshold(value);

        return curr_value;
    }

  private:
    const float threshold_size = 0.f;
    float upper_threshold = 0.f;
    float lower_threshold = 0.f;
    float curr_value = 0.f;

    void increaseThreshold(float new_upper_threshold)
    {
        upper_threshold = new_upper_threshold;
        lower_threshold = upper_threshold - threshold_size;
        curr_value = new_upper_threshold;
    }

    void decreaseThreshold(float new_lower_threshold)
    {
        lower_threshold = new_lower_threshold;
        upper_threshold = lower_threshold + threshold_size;
        curr_value = new_lower_threshold;
    }
};
