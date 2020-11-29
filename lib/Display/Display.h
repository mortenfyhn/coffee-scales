#pragma once

#include <TM1637Display.h>

class Display
{
  public:
    Display(uint8_t clk, uint8_t dio, uint8_t brightness_percent)
        : display(clk, dio)
    {
        display.clear();

        const auto brightness_zero_to_seven =
            static_cast<uint8_t>(0.07f * brightness_percent + 0.5f);

        display.setBrightness(brightness_zero_to_seven);
    }

    void clear() { display.clear(); }

    void show(const uint8_t segments[]) { display.setSegments(segments); }

    void showTime(uint8_t minutes, uint8_t seconds)
    {
        static constexpr auto colon = 0b01000000;
        display.showNumberDecEx(minutes, colon, true, 2, 0);
        display.showNumberDecEx(seconds, 0, true, 2, 2);
    }

  private:
    TM1637Display display;
};
