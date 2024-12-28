#include <Formatter.h>
#include <printf.h>  // snprintf
#include <string.h>  // strncpy

void _putchar(char) {}  // Required by printf.h, but not used.

namespace Formatter
{

constexpr uint8_t segment_off = 0b00000000;
constexpr uint8_t segment_dot = 0b10000000;
constexpr uint8_t segment_minus = 0b01000000;
constexpr uint8_t segment[] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111,  // 9
};

Buffer::buffer<char> to_string(float value)
{
    auto buf = Buffer::buffer<char>{6};

    if (snprintf(buf.data(), 6, "%5.1f", value) < 6)
        ;
    else if (snprintf(buf.data(), 6, "%4.0f", value) < 5)
        ;
    else
        strncpy(buf.data(), "----", 5);

    if (strncmp(buf.get(), " -0.0", 6) == 0)
        buf[1] = ' ';

    return buf;
}

uint8_t to_uint8(char c)
{
    return static_cast<uint8_t>(c - uint8_t{48});
}

Buffer::buffer<uint8_t> to_segments(float value)
{
    const auto string = to_string(value);

    auto segments = Buffer::buffer<uint8_t>{4};

    uint8_t i = 0;
    for (char const* c = string.get(); *c != '\0'; ++c)
    {
        if (*c == ' ')
            segments[i] = segment_off;
        else if (*c == '.')
            segments[--i] |= segment_dot;
        else if (*c == '-')
            segments[i] = segment_minus;
        else
            segments[i] = segment[to_uint8(*c)];
        i++;
    }

    return segments;
}

}  // namespace Formatter
