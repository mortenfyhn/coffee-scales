#pragma once

#include <Buffer.h>
#include <inttypes.h>

namespace Formatter
{

Buffer::buffer<char> to_string(float value);
Buffer::buffer<uint8_t> to_segments(float value);

}  // namespace Formatter
