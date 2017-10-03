#include "GramsDisplay.h"

GramsDisplay::GramsDisplay(uint8_t pin_clk, uint8_t pin_dio)
    : SevenSegmentTM1637(pin_clk, pin_dio)
{
  init();
  setBacklight(100);
}

void GramsDisplay::displayGrams(float grams)
{
  if (grams >= 10000)
    displayTooManyGrams();
  else if (grams >= 1000)
    displayWholeGrams(grams);
  else if (grams >= 0)
    displayGramsAndDecigrams(grams);
  else
    displayNegativeGrams(grams);
}

void GramsDisplay::displayTooManyGrams()
{
  print('----');
}

void GramsDisplay::displayWholeGrams(float grams)
{
  int whole_grams = int(round(grams));
  print(whole_grams);
}

void GramsDisplay::displayGramsAndDecigrams(float grams)
{
  static char decigram_string[num_digits];
  createDecigramString(decigram_string, grams);

  // Add leading zero for values below a gram (to avoid ".5")
  if (grams < 1)
    decigram_string[2] = '0';

  static uint8_t display_buffer[num_digits];
  encode(display_buffer, decigram_string, num_digits);
  addDecimalPoint(display_buffer);
  printRaw(display_buffer, num_digits, 0);
}

void GramsDisplay::createDecigramString(char* decigram_string, float grams)
{
  float decigrams = grams * 10;
  dtostrf(decigrams, num_digits, 0, decigram_string);
}

void GramsDisplay::addDecimalPoint(uint8_t* display_buffer)
{
  // Add decimal point after third digit
  display_buffer[2] += decimal_point;
}

void GramsDisplay::displayNegativeGrams(float grams)
{
  // not implemented
  displayGramsAndDecigrams(0.0);
}

