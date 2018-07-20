#ifndef HYSTERESIS_H
#define HYSTERESIS_H

class Hysteresis
{
public:
  Hysteresis(float threshold_size);
  float compute(float value);
private:
  float threshold_size;
  float upper_threshold = 0;
  float lower_threshold = 0;
  float curr_value = 0;
  void increaseThreshold(float new_upper_threshold);
  void decreaseThreshold(float new_lower_threshold);
};

#endif
