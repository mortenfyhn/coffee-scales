#pragma once

#include <Arduino.h>  // uint8_t, A6

namespace pins
{
constexpr uint8_t loadcell_dt = SCL;
constexpr uint8_t loadcell_sck = SDA;
constexpr uint8_t timer_display_clk = 7;
constexpr uint8_t timer_display_dio = MISO;
constexpr uint8_t scale_display_clk = MOSI;
constexpr uint8_t scale_display_dio = SCK;
constexpr uint8_t tare_button = 2;  // Must be interrupt pin!
constexpr uint8_t battery_voltage = A6;
}  // namespace pins
