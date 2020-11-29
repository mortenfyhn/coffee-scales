#include <Display.h>
#include <Formatter.h>
#include <HX711.h>
#include <SmoothingFilter.h>
#include <TimerDisplay.h>

// Pin connections
constexpr uint8_t hx711_dt = 2;            // D2
constexpr uint8_t hx711_sck = 4;           // D3
constexpr uint8_t timer_display_clk = 12;  // D12
constexpr uint8_t timer_display_dio = 11;  // D11
constexpr uint8_t scale_display_clk = 15;  // A1
constexpr uint8_t scale_display_dio = 16;  // A2
constexpr uint8_t tare_button = 3;         // INT0

// Configuration
constexpr float hx711_scale_factor = 1874.f;
constexpr uint8_t hx711_tare_samples = 10;
constexpr uint8_t filter_size = 10;
constexpr float hysteresis_size = 0.1f;
constexpr uint8_t brightness = 30;

auto scales = HX711{};
auto timer_display =
    TimerDisplay{timer_display_clk, timer_display_dio, brightness};
auto weight_display = Display{scale_display_clk, scale_display_dio, brightness};
auto filter = SmoothingFilter{filter_size, hysteresis_size};
volatile bool should_reset = false;

void button_pushed()
{
    should_reset = true;
}

void setup()
{
    scales.begin(hx711_dt, hx711_sck);
    scales.set_scale(hx711_scale_factor);
    scales.tare(hx711_tare_samples / 2);
    scales.tare(hx711_tare_samples / 2);

    pinMode(tare_button, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(tare_button), button_pushed, FALLING);
}

void loop()
{
    if (should_reset)
    {
        scales.tare(1);
        timer_display.stop();
        should_reset = false;
    }

    filter.addValue(scales.get_units());
    const auto weight_in_grams = filter.getValue();

    weight_display.show(Formatter::to_segments(weight_in_grams).get());

    if (weight_in_grams > 1.f)
        timer_display.start();

    timer_display.update();
}
