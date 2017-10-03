#include <HX711.h>
#include "TimerDisplay.h"
#include "GramsDisplay.h"
#include "SmoothingFilter.h"

#define DISP_TIMER_CLK 2
#define DISP_TIMER_DIO 3
#define DISP_SCALE_CLK 8
#define DISP_SCALE_DIO 9

#define FILTER_SIZE 10
#define HYSTERESIS_SIZE 0.1

#define LOAD_CELL_DT A2
#define LOAD_CELL_SCK A1
#define SCALE_FACTOR 1874
#define TARE_AVERAGES 10

HX711 loadCell(LOAD_CELL_DT, LOAD_CELL_SCK);
TimerDisplay timerDisplay(DISP_TIMER_CLK, DISP_TIMER_DIO);
GramsDisplay gramsDisplay(DISP_SCALE_CLK, DISP_SCALE_DIO);
SmoothingFilter filter(FILTER_SIZE, HYSTERESIS_SIZE);


void setup()
{
  // Serial comm
  Serial.begin(38400);

  loadCell.set_scale(SCALE_FACTOR);
  loadCell.tare(TARE_AVERAGES);
}


void loop()
{
  filter.addValue(loadCell.get_units());
  float weight_in_grams = filter.getValue();

  gramsDisplay.displayGrams(weight_in_grams);

  if (weight_in_grams > 1)
    timerDisplay.start();
  else
    timerDisplay.stop();

  timerDisplay.refresh();
}
