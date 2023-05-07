#include <Arduino.h>
#include <Display.h>
#include <Formatter.h>
#include <HX711.h>
#include <SmoothingFilter.h>
#include <TimerDisplay.h>

namespace pins
{
constexpr uint8_t loadcell_dt = 5;
constexpr uint8_t loadcell_sck = 6;
constexpr uint8_t timer_display_clk = 3;
constexpr uint8_t timer_display_dio = 4;
constexpr uint8_t scale_display_clk = 15;
constexpr uint8_t scale_display_dio = 16;
constexpr uint8_t tare_button = 10;
}  // namespace pins

namespace config
{
constexpr float scale_factor = 1874.f;
constexpr uint8_t num_tare_samples = 10;
constexpr uint8_t filter_size = 10;
constexpr float hysteresis_size = 0.1f;
constexpr uint8_t brightness = 100;
}  // namespace config

auto scales = HX711{};
auto filter = SmoothingFilter{config::filter_size, config::hysteresis_size};
auto weight_display = Display{pins::scale_display_clk, pins::scale_display_dio,
                              config::brightness};
auto timer_display = TimerDisplay{pins::timer_display_clk,
                                  pins::timer_display_dio, config::brightness};
auto tare_button_was_pushed = false;

void setup()
{
    pinMode(pins::tare_button, INPUT_PULLUP);

    scales.begin(pins::loadcell_dt, pins::loadcell_sck);
    scales.set_scale(config::scale_factor);
    scales.tare(config::num_tare_samples);
}

void loop()
{
    const auto tare_button_is_pushed = digitalRead(pins::tare_button) == LOW;
    const auto should_tare = tare_button_is_pushed && !tare_button_was_pushed;
    tare_button_was_pushed = tare_button_is_pushed;
    if (should_tare)
    {
        scales.tare(1);
        timer_display.stop();
    }

    filter.addValue(scales.get_units());
    const auto weight_in_grams = filter.getValue();

    weight_display.setSegments(Formatter::to_segments(weight_in_grams).get());

    if (weight_in_grams > 1.f)
    {
        timer_display.start();
    }

    timer_display.update();
}
