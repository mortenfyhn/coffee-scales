#include "GramsDisplay.h"

GramsDisplay::GramsDisplay(uint8_t pin_clk, uint8_t pin_dio)
    : SevenSegmentTM1637(pin_clk, pin_dio)
{
    init();
    setBacklight(100);
}

void GramsDisplay::display(float grams)
{
    if (tooLargeToDisplay(grams))
        print("----");
    else
        printGrams(grams);
}

bool GramsDisplay::tooLargeToDisplay(float grams)
{
    return (grams >= 10000 || grams <= -1000);
}

void GramsDisplay::printGrams(float grams)
{
    static char string_buffer[NUM_DIGITS + 1];
    static uint8_t display_buffer[NUM_DIGITS];

    const auto show_decigrams = shouldShowDecigrams(grams);
    createDisplayString(string_buffer, show_decigrams, grams);
    addLeadingZeroIfNeeded(string_buffer, show_decigrams, grams);
    addMinusSignIfNeeded(string_buffer, grams);
    encode(display_buffer, string_buffer, NUM_DIGITS);
    addDecimalPointIfNeeded(display_buffer, show_decigrams);
    printRaw(display_buffer);
}

bool GramsDisplay::shouldShowDecigrams(float grams)
{
    return grams > -100 && grams < 1000;
}

void GramsDisplay::createDisplayString(char* buffer, bool show_decigrams,
                                       float grams)
{
    const long display_value =
        lroundf(abs(show_decigrams ? grams * 10 : grams));
    sprintf(buffer, "%4li", display_value);
}

void GramsDisplay::addLeadingZeroIfNeeded(char* buffer, bool show_decigrams,
                                          float grams)
{
    const auto need_leading_zero = show_decigrams && abs(grams) < 1.0;
    if (need_leading_zero)
        buffer[2] = '0';
}

void GramsDisplay::addMinusSignIfNeeded(char* buffer, float grams)
{
    const auto is_negative = grams < 0;
    if (is_negative)
        buffer[0] = '-';
}

void GramsDisplay::addDecimalPointIfNeeded(uint8_t* buffer, bool show_decigrams)
{
    if (show_decigrams)
        buffer[2] += DECIMAL_POINT;
}
