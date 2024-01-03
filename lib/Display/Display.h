#pragma once

#include <TM1637Display.h>

class Display : public TM1637Display
{
  public:
    Display(uint8_t pin_clk, uint8_t pin_dio) : TM1637Display(pin_clk, pin_dio)
    {
        clear();
        setMaxBrightness();
    }

    void showLine()
    {
        constexpr uint8_t segments_line[] = {SEG_G, SEG_G, SEG_G, SEG_G};

        setSegments(segments_line);
    }

    void setMaxBrightness() { setBrightness(7); }
    void setMinBrightness() { setBrightness(0); }
};
