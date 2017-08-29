#ifndef GRAMS_DISPLAY_H
#define GRAMS_DISPLAY_H

#include <SevenSegmentTM1637.h>

class GramsDisplay
{
public:
  GramsDisplay(uint8_t pin_clk, uint8_t pin_dio);
  void displayGrams(float grams);
private:
  SevenSegmentTM1637 display;
  static const int num_digits = TM1637_MAX_COLOM;
  static const uint8_t decimal_point = 0x80;
  void displayTooManyGrams();
  void displayWholeGrams(float grams);
  void displayGramsAndDecigrams(float grams);
  void createDecigramString(char* decigram_string, float grams);
  void addDecimalPoint(uint8_t* display_buffer);
  void displayNegativeGrams(float grams);
  void printZero();
};

#endif
