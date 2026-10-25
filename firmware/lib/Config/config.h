#pragma once

#include <stdint.h>

namespace config
{
// To calibrate the division factor:
//   division factor = old division factor * measured / true
// where 'measured' and 'true' are the measured and true weights of an item.
constexpr float division_factor = 1540.32;
constexpr uint8_t num_tare_samples = 10;
constexpr uint8_t filter_size = 10;
constexpr float hysteresis_size = 0.02;
constexpr float battery_scaling = 2.0 * 3.3 / 1024.0;
constexpr uint32_t tare_interval_ms = 1000;
constexpr uint32_t timeout_dim_ms = 1 * 60 * 1000l;    // 1 min
constexpr uint32_t timeout_sleep_ms = 5 * 60 * 1000l;  // 5 min
constexpr float zero_tracking_band = 0.3;
}  // namespace config
