#include "button.h"
#include "pins.h"
#include "serial.h"
#include <Arduino.h>
#include <Display.h>
#include <Formatter.h>
#include <HX711.h>
#include <Scale.h>
#include <TimerDisplay.h>
#include <avr/sleep.h>
#include <config.h>
#include <stdint.h>

auto load_cell_ = HX711{};
auto weight_display_ =
    Display{pins::scale_display_clk, pins::scale_display_dio};
auto timer_display_ =
    TimerDisplay{pins::timer_display_clk, pins::timer_display_dio};
auto button_ = Button{pins::tare_button};
LogRow log_row_;

float read_battery_voltage()
{
    return config::battery_scaling * analogRead(pins::battery_voltage);
}

void attachTareButtonInterrupt()
{
    attachInterrupt(
        digitalPinToInterrupt(pins::tare_button),
        [] { button_.interrupt_handler(); }, FALLING);
}

void detachTareButtonInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(pins::tare_button));
}

class DeviceHardware : public Hardware
{
  public:
    uint32_t millis() override { return ::millis(); }

    int32_t read_load_cell() override
    {
        const uint32_t wait_start_us = log_micros();
        const int32_t raw_value = load_cell_.read();
        log_row_.has_raw = true;
        log_row_.raw = raw_value;
        log_row_.wait_us = log_micros() - wait_start_us;
        return raw_value;
    }

    bool tare_pressed() override
    {
        log_row_.pressed = button_.is_pressed();
        return log_row_.pressed;
    }

    bool long_pressed() override
    {
        log_row_.long_pressed = button_.is_long_pressed();
        return log_row_.long_pressed;
    }

    void show_line() override { weight_display_.showLine(); }

    void show_weight(float grams) override
    {
        weight_display_.setSegments(Formatter::to_segments(grams).get());
        log_row_.has_weight = true;
        log_row_.weight_x10 = lroundf(grams * 10);
    }

    void set_brightness_max() override
    {
        weight_display_.setBrightnessMax();
        timer_display_.setBrightnessMax();
    }

    void set_brightness_dim() override
    {
        weight_display_.setBrightnessDim();
        timer_display_.setBrightnessDim();
    }

    void start_timer() override { timer_display_.start(); }
    void stop_timer() override { timer_display_.stop(); }
    void update_timer() override { timer_display_.update(); }

    void sleep() override
    {
        weight_display_.clear();
        timer_display_.stop();
        load_cell_.power_down();

        // Turn off red indicator LED that I stupidly tied the scale display's DIO pin to...
        digitalWrite(13, LOW);

        // When sleep mode is triggered by a long press, we must disable the
        // interrupt for a moment, so that letting go of the button doesn't
        // trigger the ISR and wake the device.
        detachTareButtonInterrupt();
        delay(1000ul);
        attachTareButtonInterrupt();

        sleep_mode();

        load_cell_.power_up();
    }
};

auto hardware_ = DeviceHardware{};

#ifdef LOGGING
auto scale_ = Scale{hardware_, false};  // Don't sleep when logging
#else
auto scale_ = Scale{hardware_, true};
#endif

void setup()
{
    serial_setup();
    debug("setup");

    // Configure button
    pinMode(pins::tare_button, INPUT_PULLUP);
    attachTareButtonInterrupt();

    // Enable power-down sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    // Set up the load cell
    load_cell_.begin(pins::loadcell_dt, pins::loadcell_sck);

    // Print battery voltage
    auto buf = Buffer::buffer<char>{5};
    const auto battery_voltage_mV =
        static_cast<uint32_t>(read_battery_voltage() * 1000);
    snprintf(buf.data(), 5, "%lu", battery_voltage_mV);
    for (auto i = 0; i < 5; ++i)
        buf.data()[i] = weight_display_.encode(buf.data()[i]);
    buf.data()[0] |= 0b10000000;  // Add decimal dot
    weight_display_.printRaw(buf.data());

    delay(1000ul);

    scale_.start();
}

void loop()
{
    log_row_.time_ms = millis();

    const State prev_state = scale_.state();
    scale_.step();
    if (scale_.state() != prev_state)
    {
        constexpr const char* names[] = {"taring", "ready", "brewing", "dim",
                                         "sleep"};
        debug(names[static_cast<uint8_t>(scale_.state())]);
    }

    log_row_.state = static_cast<uint8_t>(scale_.state());
    log(log_row_);
    log_row_ = {};
}
