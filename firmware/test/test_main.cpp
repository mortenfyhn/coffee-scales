#include <Formatter.h>
#include <unity.h>

constexpr uint8_t off = 0x00;
constexpr uint8_t dot = 0x80;
constexpr uint8_t minus = 0x40;
constexpr uint8_t zero = 0x3f;
constexpr uint8_t one = 0x06;
constexpr uint8_t two = 0x5b;
constexpr uint8_t three = 0x4f;
constexpr uint8_t four = 0x66;
constexpr uint8_t five = 0x6d;
constexpr uint8_t six = 0x7d;
constexpr uint8_t seven = 0x07;
constexpr uint8_t eight = 0x7f;
constexpr uint8_t nine = 0x6f;

Buffer::buffer<uint8_t> segments(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    auto buffer = Buffer::buffer<uint8_t>{4};
    buffer[0] = a;
    buffer[1] = b;
    buffer[2] = c;
    buffer[3] = d;
    return buffer;
}

void test(float value, const char* string,
          Buffer::buffer<uint8_t> const& segments)
{
    TEST_ASSERT_EQUAL_STRING(string, Formatter::to_string(value).get());

    TEST_ASSERT_EQUAL_HEX8_ARRAY(segments.get(),
                                 Formatter::to_segments(value).get(), 4);
}

void positive_integers()
{
    test(0.f, "  0.0", segments(off, off, zero | dot, zero));
    test(1.f, "  1.0", segments(off, off, one | dot, zero));
    test(10.f, " 10.0", segments(off, one, zero | dot, zero));
    test(100.f, "100.0", segments(one, zero, zero | dot, zero));
    test(1000.f, "1000", segments(one, zero, zero, zero));
    test(9999.f, "9999", segments(nine, nine, nine, nine));
}

void negative_integers()
{
    test(-0.f, "  0.0", segments(off, off, zero | dot, zero));
    test(-1.f, " -1.0", segments(off, minus, one | dot, zero));
    test(-10.f, "-10.0", segments(minus, one, zero | dot, zero));
    test(-100.f, "-100", segments(minus, one, zero, zero));
    test(-999.f, "-999", segments(minus, nine, nine, nine));
}

void out_of_bounds()
{
    test(10000.f, "----", segments(minus, minus, minus, minus));
    test(-1000.f, "----", segments(minus, minus, minus, minus));
}

void rounding()
{
    test(0.05f, "  0.1", segments(off, off, zero | dot, one));
    test(-0.05f, " -0.1", segments(off, minus, zero | dot, one));
    test(1000.6, "1001", segments(one, zero, zero, one));
    test(-100.6, "-101", segments(minus, one, zero, one));
}

void negative_zero()
{
    test(-0.01f, "  0.0", segments(off, off, zero | dot, zero));
    test(-0.1f, " -0.1", segments(off, minus, zero | dot, one));
}

void setup() {}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(positive_integers);
    RUN_TEST(negative_integers);
    RUN_TEST(out_of_bounds);
    RUN_TEST(rounding);
    RUN_TEST(negative_zero);
    UNITY_END();
}
