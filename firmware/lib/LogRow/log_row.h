#pragma once

#include <stdint.h>

// One CSV row per loop iteration. Contains the loop's inputs and outputs, so a
// log can be replayed on the host. Optional fields are left empty if not set.
struct LogRow
{
    uint32_t time_ms = 0;
    bool has_raw = false;
    int32_t raw = 0;
    uint32_t wait_us = 0;  // How long the HX711 read waited for a sample
    bool pressed = false;
    bool long_pressed = false;
    uint8_t state = 0;
    bool has_weight = false;
    int32_t weight_x10 = 0;  // Displayed weight in tenths of a gram
    uint32_t press_ms = 0;   // Time of the button press, if pressed
};

constexpr const char* log_header =
    "time_ms,raw,wait_us,pressed,long_pressed,state,weight_x10,press_ms";
