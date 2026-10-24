#pragma once

#include <math.h>

class Hysteresis
{
  public:
    Hysteresis(float margin) : margin(margin) {}

    float compute(float input)
    {
        if (input > output + margin + 0.05f || input < output - margin - 0.05f)
            output = roundf(input * 10) / 10;

        return output;
    }

    void reset() { output = 0.f; }

  private:
    float output = 0.f;
    const float margin;
};
