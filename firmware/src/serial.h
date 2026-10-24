#pragma once

#include <Arduino.h>
#include <log_row.h>

void serial_setup()
{
#ifdef LOGGING
    Serial.begin(38400);
#endif

#ifdef DEBUG
    Serial.begin(38400);
#endif

#ifdef LOGGING
    Serial.println(log_header);
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
    Serial.print(',');
    if (row.pressed)
        Serial.print(row.press_ms);
    Serial.println();
#else
    (void)row;  // Silence the unused parameter warning
#endif
}
