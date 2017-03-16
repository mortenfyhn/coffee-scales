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

#define NUM_DIGITS 4
#define FILTER_SIZE 10
#define BRIGHTNESS 100

#define SCALE_FACTOR 1876
#define SCALE_OFFSET 105193 - 100

SevenSegmentExtended timerDisp(DISP_TIMER_CLK, DISP_TIMER_DIO);
SevenSegmentTM1637   scaleDisp(DISP_SCALE_CLK, DISP_SCALE_DIO);
HX711                scale;
RunningAverage       filter(FILTER_SIZE);
char                 decigrams_array[NUM_DIGITS];
uint8_t              display_buffer[NUM_DIGITS];
float                weight_in_grams;
uint32_t             start_time;
bool                 has_started = false;


void setup() {
  // Serial comm
  Serial.begin(38400);

  // Timer display
  timerDisp.begin();
  timerDisp.setBacklight(BRIGHTNESS);

  // Scale display
  scaleDisp.begin();
  scaleDisp.setBacklight(BRIGHTNESS);

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
  printGrams(scaleDisp, weight_in_grams);

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



void printGrams(SevenSegmentTM1637 &disp, float grams)
{
  if (grams < 0)
  {
    // Handle negative values better
    disp.clear();
    disp.print(grams);
  }
  else if (grams > 9999)
  {
    disp.print("----");
  }
  else if (grams >= 1000)
  {
    int grams_int = int(round(grams));
    disp.clear();
    disp.print(grams_int);
  }
  else if (grams < 1000)
  {
    // Assemble char array for the display's digits
    float decigrams = grams * 10;
    dtostrf(decigrams, NUM_DIGITS, 0, decigrams_array);

    // Add leading zero for values below a gram (to avoid ".5")
    if (grams < 1)
      decigrams_array[2] = '0';

    // Encode to byte array
    disp.encode(display_buffer, decigrams_array, NUM_DIGITS);

    // Add decimal point after third digit
    display_buffer[2] += 0b10000000;

    // Print to display
    disp.printRaw(display_buffer, NUM_DIGITS, 0);
  }
}
