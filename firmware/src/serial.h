#pragma once

#include <Arduino.h>

// One CSV row per loop iteration. Contains the loop's inputs and outputs, so a
// log can be replayed on the host. Optional fields are left empty if not set.
struct LogRow
{
    uint32_t time_ms = 0;
    bool has_raw = false;
    int32_t raw = 0;
    uint32_t wait_us = 0;  // How long read() waited for the HX711
    bool pressed = false;
    bool long_pressed = false;
    uint8_t state = 0;
    bool has_weight = false;
    int32_t weight_x10 = 0;  // Displayed weight in tenths of a gram
};

void serial_setup()
{
#ifdef LOGGING
    Serial.begin(38400);
#endif

#ifdef DEBUG
    Serial.begin(38400);
#endif

#ifdef LOGGING
    Serial.println("time_ms,raw,wait_us,pressed,long_pressed,state,weight_x10");
#endif
}

template <typename T> void debug(const T& message)
{
#ifdef DEBUG
    Serial.println(message);
#else
    (void)message;  // Silence the unused parameter warning
#endif
}

// Returns micros() in the logging build. Returns 0 otherwise, so the release
// build doesn't pay for timing it doesn't log.
uint32_t log_micros()
{
#ifdef LOGGING
    return micros();
#else
    return 0;
#endif
}

void log(const LogRow& row)
{
#ifdef LOGGING
    // Print integers only, since float formatting is slow on the AVR.
    Serial.print(row.time_ms);
    Serial.print(',');
    if (row.has_raw)
    {
        Serial.print(row.raw);
        Serial.print(',');
        Serial.print(row.wait_us);
    }
    else
    {
        Serial.print(',');
    }
    Serial.print(',');
    Serial.print(row.pressed);
    Serial.print(',');
    Serial.print(row.long_pressed);
    Serial.print(',');
    Serial.print(row.state);
    Serial.print(',');
    if (row.has_weight)
        Serial.print(row.weight_x10);
    Serial.println();
#else
    (void)row;  // Silence the unused parameter warning
#endif
}
