#include <GramsDisplay.h>
#include <HX711.h>
#include <SmoothingFilter.h>
#include <TimerDisplay.h>

// Pin connections
constexpr auto hx711_dt = 2;            // D2
constexpr auto hx711_sck = 3;           // D3
constexpr auto timer_display_clk = 12;  // D12
constexpr auto timer_display_dio = 11;  // D11
constexpr auto scale_display_clk = 15;  // A1
constexpr auto scale_display_dio = 16;  // A2

// Settings
constexpr auto hx711_scale_factor = 1874.f;
constexpr auto hx711_tare_samples = 10;
constexpr auto filter_size = 10;
constexpr auto hysteresis_size = 0.1f;

auto loadCell = HX711{};
auto timerDisplay = TimerDisplay{timer_display_clk, timer_display_dio};
auto weightDisplay = GramsDisplay{scale_display_clk, scale_display_dio};
auto filter = SmoothingFilter{filter_size, hysteresis_size};

void setup()
{
    loadCell.begin(hx711_dt, hx711_sck);
    loadCell.set_scale(hx711_scale_factor);
    loadCell.tare(hx711_tare_samples / 2);
    loadCell.tare(hx711_tare_samples / 2);
}

void loop()
{
    filter.addValue(loadCell.get_units());
    const auto weight_in_grams = filter.getValue();

    weightDisplay.display(weight_in_grams);

    if (weight_in_grams > 1.f)
        timerDisplay.start();

    timerDisplay.update();
}
