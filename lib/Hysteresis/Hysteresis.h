#pragma once

class Hysteresis
{
  public:
    Hysteresis(float threshold) : threshold(threshold) {}

    float compute(float input)
    {
        if (input > output + threshold || input < output - threshold)
            output = input;

        return output;
    }

  private:
    float output = 0.f;
    const float threshold;
};
