#pragma once

#include <TM1637Display.h>

class Display : public TM1637Display
{
  public:
    Display(uint8_t pin_clk, uint8_t pin_dio, uint8_t brightness_percent)
        : TM1637Display(pin_clk, pin_dio)
    {
        clear();

        const auto brightness_zero_to_seven =
            static_cast<uint8_t>(0.07f * brightness_percent + 0.5f);

        setBrightness(brightness_zero_to_seven);
    }

    void showLine()
    {
        constexpr uint8_t segments_line[] = {SEG_G, SEG_G, SEG_G, SEG_G};

        setSegments(segments_line);
    }
};
