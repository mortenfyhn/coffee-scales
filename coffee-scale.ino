#include <RunningAverage.h>
#include <HX711.h>

#include "TimerDisplay.h"
#include "GramsDisplay.h"

#define DISP_TIMER_CLK 2
#define DISP_TIMER_DIO 3
#define DISP_SCALE_CLK 8
#define DISP_SCALE_DIO 9
#define SCALE_DT       A2
#define SCALE_SCK      A1

#define FILTER_SIZE 10

#define SCALE_FACTOR 1874
#define SCALE_OFFSET 984550

TimerDisplay         timerDisplay(DISP_TIMER_CLK, DISP_TIMER_DIO);
GramsDisplay         gramsDisplay(DISP_SCALE_CLK, DISP_SCALE_DIO);
HX711                scale;
RunningAverage       filter(FILTER_SIZE);
float                weight_in_grams;

void setup()
{
  // Serial comm
  Serial.begin(38400);

  // Load cell
  scale.begin(SCALE_DT, SCALE_SCK);
  scale.set_scale(SCALE_FACTOR);
  scale.set_offset(SCALE_OFFSET);

  // Filter
  filter.clear();
}


void loop()
{
  filter.addValue(scale.get_units());
  weight_in_grams = filter.getAverage();
  gramsDisplay.displayGrams(weight_in_grams);

  if (weight_in_grams > 1)
    timerDisplay.start();
  else
    timerDisplay.stop();

  timerDisplay.refresh();
}
