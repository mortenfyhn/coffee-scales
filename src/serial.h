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

void debug(const String& message)
{
#ifdef DEBUG
    Serial.println(message);
#else
    (void)message;  // Silence the unused parameter warning
#endif
}
