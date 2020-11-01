#ifndef GRAMS_DISPLAY_H
#define GRAMS_DISPLAY_H

#include <SevenSegmentTM1637.h>

class GramsDisplay : public SevenSegmentTM1637
{
  public:
    GramsDisplay(uint8_t pin_clk, uint8_t pin_dio);
    void display(float grams);

  private:
    static constexpr int NUM_DIGITS = TM1637_MAX_COLOM;
    static constexpr uint8_t DECIMAL_POINT = 0x80;
    static constexpr uint8_t MINUS_SIGN = 0x40;

    static bool tooLargeToDisplay(float grams);
    void printGrams(float grams);
    static bool shouldShowDecigrams(float grams);
    static void createDisplayString(char* buffer, bool show_decigrams,
                                    float grams);
    static void addLeadingZeroIfNeeded(char* buffer, bool show_decigrams,
                                       float grams);
    static void addMinusSignIfNeeded(char* buffer, float grams);
    static void addDecimalPointIfNeeded(uint8_t* buffer, bool show_decigrams);
};

#endif
