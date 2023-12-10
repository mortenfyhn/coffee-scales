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

    void showTare()
    {
        constexpr uint8_t segments_tare[] = {
            SEG_D | SEG_E | SEG_F | SEG_G,                  // t
            SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
            SEG_E | SEG_G,                                  // r
            SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
        };

        setSegments(segments_tare);
    }
};
