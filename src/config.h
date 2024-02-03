#pragma once

#include <Arduino.h>  // uint8_t, A1

namespace pins
{
constexpr uint8_t loadcell_dt = 4;
constexpr uint8_t loadcell_sck = 3;
constexpr uint8_t timer_display_clk = 6;
constexpr uint8_t timer_display_dio = 9;
constexpr uint8_t scale_display_clk = A1;
constexpr uint8_t scale_display_dio = A2;
constexpr uint8_t tare_button = 2;  // Must be interrupt compatible!
constexpr uint8_t battery_voltage = A6;
}  // namespace pins

namespace config
{
constexpr float scale_factor = 1574.f;
constexpr uint8_t num_tare_samples = 10;
constexpr uint8_t filter_size = 10;
constexpr float hysteresis_size = 0.1f;
constexpr auto battery_scaling = 2.f * 3.3f / 1024.f;
constexpr auto tare_interval_ms = 1000ul;
constexpr auto timeout_dim_ms = 1ul * 60ul * 1000ul;    // 1 min
constexpr auto timeout_sleep_ms = 5ul * 60ul * 1000ul;  // 5 min
}  // namespace config
