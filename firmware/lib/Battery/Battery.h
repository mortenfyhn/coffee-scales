#pragma once

namespace battery
{

inline int percentage(float voltage)
{
    if (voltage >= 4.20f)
        return 100;
    if (voltage >= 3.85f)
        return 75 + (voltage - 3.85f) * (100 - 75) / (4.20 - 3.85) + 0.5;
    if (voltage >= 3.78f)
        return 50 + (voltage - 3.78f) * (75 - 50) / (3.85 - 3.78) + 0.5;
    if (voltage >= 3.73f)
        return 25 + (voltage - 3.73f) * (50 - 25) / (3.78 - 3.73) + 0.5;
    if (voltage >= 3.70f)
        return 0 + (voltage - 3.70f) * (25 - 0) / (3.73 - 3.70) + 0.5;
    return 0;
}

}  // namespace battery