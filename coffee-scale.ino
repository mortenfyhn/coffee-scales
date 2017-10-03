#include <HX711.h>
#include "TimerDisplay.h"
#include "GramsDisplay.h"
#include "SmoothingFilter.h"

#define DISP_TIMER_CLK 2
#define DISP_TIMER_DIO 3
#define DISP_SCALE_CLK 8
#define DISP_SCALE_DIO 9
#define SCALE_DT       A2
#define SCALE_SCK      A1

#define FILTER_SIZE 10
#define HYSTERESIS_SIZE 0.1

#define SCALE_FACTOR 1874
#define SCALE_OFFSET 984550
#define TARE_AVERAGES 10

TimerDisplay    timerDisplay(DISP_TIMER_CLK, DISP_TIMER_DIO);
GramsDisplay    gramsDisplay(DISP_SCALE_CLK, DISP_SCALE_DIO);
HX711           scale;
SmoothingFilter filter(FILTER_SIZE, HYSTERESIS_SIZE);


void setup()
{
  // Serial comm
  Serial.begin(38400);

  // Load cell
  scale.begin(SCALE_DT, SCALE_SCK);
  scale.set_scale(SCALE_FACTOR);
  scale.tare(TARE_AVERAGES);
}


void loop()
{
  filter.addValue(scale.get_units());
  float weight_in_grams = filter.getValue();

  gramsDisplay.displayGrams(weight_in_grams);

  if (weight_in_grams > 1)
    timerDisplay.start();
  else
    timerDisplay.stop();

  timerDisplay.refresh();
}
