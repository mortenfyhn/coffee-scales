#include <Arduino.h>
#include <Display.h>
#include <Formatter.h>
#include <HX711.h>
#include <Hysteresis.h>
#include <SmoothingFilter.h>
#include <TimerDisplay.h>
#include <avr/sleep.h>

namespace pins
{
constexpr uint8_t loadcell_dt = 4;
constexpr uint8_t loadcell_sck = 3;
constexpr uint8_t timer_display_clk = 6;
constexpr uint8_t timer_display_dio = 9;
constexpr uint8_t scale_display_clk = A1;
constexpr uint8_t scale_display_dio = A2;
constexpr uint8_t tare_button = 2;  // Must be interrupt compatible!
constexpr uint8_t battery_voltage = A6;
constexpr uint8_t low_battery_lamp = SCK;
}  // namespace pins

namespace config
{
constexpr float scale_factor = 1574.f;
constexpr uint8_t num_tare_samples = 10;
constexpr uint8_t filter_size = 10;
constexpr float hysteresis_size = 0.1f;
constexpr uint8_t brightness = 100;
constexpr auto battery_scaling = 2.f * 3.3f / 1024.f;
constexpr auto low_battery_limit_v = 3.7f;
constexpr auto tare_interval_ms = 1000ul;
constexpr auto inactivity_timeout_ms = 300000ul;  // 5 min (1 min = 60 000 ms)
}  // namespace config

class Taring
{
  public:
    void request()
    {
        // This debounces without waiting.

        const auto curr_time_ms = millis();

        // If prev time is zero, then this is the first request, which we should
        // always honour.
        if (prev_time_ms_ == 0ul ||
            curr_time_ms - prev_time_ms_ > config::tare_interval_ms)
        {
            requested_ = true;
            prev_time_ms_ = curr_time_ms;
        }
    }

    bool should_tare()
    {
        if (requested_)
        {
            requested_ = false;
            return true;
        }

        return false;
    }

  private:
    bool requested_ = false;
    unsigned long prev_time_ms_ = 0ul;
};

auto scales = HX711{};
auto filter = SmoothingFilter{config::filter_size};
auto hysteresis = Hysteresis{config::hysteresis_size};
auto weight_display = Display{pins::scale_display_clk, pins::scale_display_dio,
                              config::brightness};
auto timer_display = TimerDisplay{pins::timer_display_clk,
                                  pins::timer_display_dio, config::brightness};
auto taring = Taring{};
auto last_activity_time_ms = 0ul;

void setup()
{
#ifdef LOGGING
    Serial.begin(38400);
    Serial.println("time,data");  // CSV header
#endif

    pinMode(pins::tare_button, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(pins::tare_button), [] { taring.request(); },
        FALLING);
    pinMode(pins::low_battery_lamp, OUTPUT);

    // Enable power-down sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    scales.begin(pins::loadcell_dt, pins::loadcell_sck);
    scales.set_scale(config::scale_factor);
    taring.request();
}

void loop()
{
    // Taring
    if (taring.should_tare())
    {
        scales.power_up();
        timer_display.stop();

        // Wait until the filter is stable, for accurate taring
        while (!filter.hasSteadyState())
        {
            weight_display.showLine();

            const auto raw_value = scales.read();
            const auto weight_in_grams_raw =
                (raw_value - scales.get_offset()) / config::scale_factor;
            filter.addValue(weight_in_grams_raw);
        }

        const auto new_offset =
            scales.get_offset() + filter.getValue() * config::scale_factor;
        scales.set_offset(new_offset);
        hysteresis.reset();
        last_activity_time_ms = millis();
    }

    // Read weight
    const auto raw_value = scales.read();
    const auto weight_in_grams_raw =
        (raw_value - scales.get_offset()) / config::scale_factor;

    // Filter weight
    filter.addValue(weight_in_grams_raw);
    const auto weight_in_grams = hysteresis.compute(filter.getValue());

    if (!filter.hasSteadyState())
    {
        last_activity_time_ms = millis();
    }

    // Display weight
    weight_display.setSegments(Formatter::to_segments(weight_in_grams).get());

    // Display brew timer
    if (weight_in_grams > 1.f)
    {
        timer_display.start();
    }
    timer_display.update();

    // Low battery warning
    const auto battery_voltage_v =
        config::battery_scaling * analogRead(pins::battery_voltage);
    if (battery_voltage_v < config::low_battery_limit_v)
    {
        digitalWrite(pins::low_battery_lamp, HIGH);
    }

    // Go to sleep if idle for too long
    const auto time_now_ms = millis();
    if (time_now_ms - last_activity_time_ms > config::inactivity_timeout_ms)
    {
        weight_display.clear();
        timer_display.stop();
        filter.clear();
        scales.power_down();
        sleep_mode();
    }

#ifdef LOGGING
    Serial.print(millis() / 1000.0);
    Serial.print(",");
    Serial.println(raw_value);
#endif
}
