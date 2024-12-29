#pragma once

#include <SevenSegmentTM1637.h>

class Display : public SevenSegmentTM1637
{
  public:
    Display(uint8_t pin_clk, uint8_t pin_dio)
        : SevenSegmentTM1637(pin_clk, pin_dio)
    {
        init();
        setBrightnessMax();
    }

    void showLine()
    {
        constexpr uint8_t segments_line[] = {SEG_G, SEG_G, SEG_G, SEG_G};

        setSegments(segments_line);
    }

    void showSomething()
    {
        constexpr uint8_t segments_line[] = {SEG_G, SEG_G};

        setSegments(segments_line);
    }

    void setSegments(const uint8_t* data, size_t length = 4,
                     uint8_t position = 0)
    {
        printRaw(data, length, position);
    }

    void setBrightnessMax() { setBacklight(100); }
    void setBrightnessDim() { setBacklight(10); }

  private:
    static constexpr uint8_t SEG_G = 0b01000000;
};
