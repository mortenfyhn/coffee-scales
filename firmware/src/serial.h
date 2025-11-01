#pragma once

#include <Arduino.h>

void serial_setup()
{
#ifdef LOGGING
    Serial.begin(38400);
#endif

#ifdef DEBUG
    Serial.begin(38400);
#endif

#ifdef LOGGING
    Serial.println("time,data");  // CSV header
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

template <typename T> void log(const T& message)
{
#ifdef LOGGING
    const auto seconds = static_cast<float>(millis()) / 1000;
    Serial.println(String(seconds) + "," + message);
#else
    (void)message;  // Silence the unused parameter warning
#endif
}
