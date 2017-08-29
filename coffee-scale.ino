#include <RunningAverage.h>
#include <SevenSegmentExtended.h>
#include <TimeLib.h>
#include <HX711.h>
#include <stdlib.h>

#define DISP_TIMER_CLK 12
#define DISP_TIMER_DIO 11
#define DISP_SCALE_CLK 3
#define DISP_SCALE_DIO 2
#define SCALE_DT       A1
#define SCALE_SCK      A2
#include "GramsDisplay.h"


#define NUM_DIGITS 4
#define FILTER_SIZE 10
#define BRIGHTNESS 100

#define SCALE_FACTOR 1876
#define SCALE_OFFSET 105193 - 100

SevenSegmentExtended timerDisp(DISP_TIMER_CLK, DISP_TIMER_DIO);
GramsDisplay         gramsDisplay(DISP_SCALE_CLK, DISP_SCALE_DIO);
HX711                scale;
RunningAverage       filter(FILTER_SIZE);
float                weight_in_grams;
uint32_t             start_time;
bool                 has_started = false;


void setup() {
  // Serial comm
  Serial.begin(38400);

  // Timer display
  timerDisp.begin();
  timerDisp.setBacklight(BRIGHTNESS);

  // Load cell
  scale.begin(SCALE_DT, SCALE_SCK);
  scale.set_scale(SCALE_FACTOR);
  scale.set_offset(SCALE_OFFSET);

  // Filter
  filter.clear();
}



void loop() {
  // Scale
  filter.addValue(scale.get_units());
  weight_in_grams = filter.getAverage();
  gramsDisplay.displayGrams(weight_in_grams);

  // Timer
  if (has_started == true)
  {
    timerDisp.printTime(minute(), second(), false);
  }
  else if (weight_in_grams > 1)
  {
    has_started = true;
    // setTime(0);
    timerDisp.printTime(minute(), second(), false);
  }
  Serial.println(has_started);
}
