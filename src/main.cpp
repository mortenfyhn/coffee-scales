#include <Formatter.h>
#include <HX711.h>
#include <SmoothingFilter.h>
#include <TM1637Display.h>
#include <TimerDisplay.h>

// Pin connections
constexpr auto hx711_dt = 2;            // D2
constexpr auto hx711_sck = 3;           // D3
constexpr auto timer_display_clk = 12;  // D12
constexpr auto timer_display_dio = 11;  // D11
constexpr auto scale_display_clk = 15;  // A1
constexpr auto scale_display_dio = 16;  // A2

// Configuration
constexpr auto hx711_scale_factor = 1874.f;
constexpr auto hx711_tare_samples = 10;
constexpr auto filter_size = 10;
constexpr auto hysteresis_size = 0.1f;

auto scales = HX711{};
auto timer_display = TimerDisplay{timer_display_clk, timer_display_dio};
auto weight_display = TM1637Display{scale_display_clk, scale_display_dio};
auto filter = SmoothingFilter{filter_size, hysteresis_size};

void setup()
{
    weight_display.setBrightness(100);

    scales.begin(hx711_dt, hx711_sck);
    scales.set_scale(hx711_scale_factor);
    scales.tare(hx711_tare_samples / 2);
    scales.tare(hx711_tare_samples / 2);
}

void loop()
{
    filter.addValue(scales.get_units());
    const auto weight_in_grams = filter.getValue();

    weight_display.setSegments(Formatter::to_segments(weight_in_grams).get(),
                               4);

    if (weight_in_grams > 1.f)
        timer_display.start();

    timer_display.update();
}
