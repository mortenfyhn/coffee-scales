#include "button.h"
#include "config.h"
#include "serial.h"
#include <Arduino.h>
#include <Display.h>
#include <Formatter.h>
#include <HX711.h>
#include <Hysteresis.h>
#include <SmoothingFilter.h>
#include <TimerDisplay.h>
#include <avr/sleep.h>
#include <stdint.h>

enum class State
{
    taring,
    ready,
    brewing,
    dim,
    sleep,
} state_ = State::taring;

auto load_cell_ = HX711{};
auto filter_ = SmoothingFilter{config::filter_size};
auto hysteresis_ = Hysteresis{config::hysteresis_size};
auto weight_display_ =
    Display{pins::scale_display_clk, pins::scale_display_dio};
auto timer_display_ =
    TimerDisplay{pins::timer_display_clk, pins::timer_display_dio};
auto button_ = Button{pins::tare_button};
uint32_t last_activity_time_ms_ = 0;
float bias_ = 0.f;
float prev_filtered_ = 0.f;
float bias_compensated_ = 0.f;

float read_battery_voltage()
{
    return config::battery_scaling * analogRead(pins::battery_voltage);
}

void read_load_cell_and_update_filter()
{
    const int32_t raw_value = load_cell_.read();
    log(raw_value);
    const float weight_in_grams_raw =
        (raw_value - load_cell_.get_offset()) / config::division_factor;
    filter_.addValue(weight_in_grams_raw);

    const auto curr_filtered = filter_.getValue();
    if (filter_.hasSteadyState())
    {
        bias_ += curr_filtered - prev_filtered_;
    }
    bias_compensated_ = curr_filtered - bias_;
    prev_filtered_ = curr_filtered;
}

void tare()
{
    const int32_t new_offset =
        load_cell_.get_offset() + filter_.getValue() * config::division_factor;
    load_cell_.set_offset(new_offset);
    hysteresis_.reset();

    bias_ = 0.f;
    prev_filtered_ = 0.f;
    bias_compensated_ = 0.f;
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

//--- TRANSITIONS --------------------------------------------------------------

[[nodiscard]] State transitionToTaring()
{
    debug("-> taring");

    weight_display_.setBrightnessMax();
    timer_display_.setBrightnessMax();

    timer_display_.stop();
    last_activity_time_ms_ = millis();

    return State::taring;
}

[[nodiscard]] State transitionToReady()
{
    debug(" -> ready");

    weight_display_.setBrightnessMax();
    timer_display_.setBrightnessMax();

    return State::ready;
}

[[nodiscard]] State transitionToBrewing()
{
    debug("  -> brewing");

    weight_display_.setBrightnessMax();
    timer_display_.setBrightnessMax();

    timer_display_.start();

    return State::brewing;
}

[[nodiscard]] State transitionToDim()
{
    debug("   -> dim");

    weight_display_.setBrightnessDim();
    timer_display_.setBrightnessDim();

    return State::dim;
}

[[nodiscard]] State transitionToSleep()
{
    debug("    -> sleep");

    return State::sleep;
}

//--- STATES -------------------------------------------------------------------

[[nodiscard]] State taring()
{
    // I do this to clear any pending "should tares".
    // TODO: Figure out if needed, if not remove, if yes refactor.
    button_.is_pressed();

    // Placing this check early makes taring instant when the filter is already
    // stable, without wasting time on showing the "wait for taring" line.
    if (filter_.hasSteadyState())
    {
        tare();
        return transitionToReady();
    }

    weight_display_.showLine();

    read_load_cell_and_update_filter();

    return State::taring;
}

[[nodiscard]] State ready()
{
    if (button_.is_pressed())
    {
        debug("should tare");
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const float weight_in_grams = hysteresis_.compute(bias_compensated_);
    weight_display_.setSegments(Formatter::to_segments(weight_in_grams).get());

    if (!filter_.hasSteadyState())
    {
        last_activity_time_ms_ = millis();
    }

    if (weight_in_grams > 1.f)
    {
        return transitionToBrewing();
    }

    if (millis() - last_activity_time_ms_ > config::timeout_dim_ms)
    {
        return transitionToDim();
    }

    return State::ready;
}

[[nodiscard]] State brewing()
{
    if (button_.is_pressed())
    {
        debug("should tare");
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const float weight_in_grams = hysteresis_.compute(bias_compensated_);
    weight_display_.setSegments(Formatter::to_segments(weight_in_grams).get());
    timer_display_.update();

    if (!filter_.hasSteadyState())
    {
        last_activity_time_ms_ = millis();
    }

    if (millis() - last_activity_time_ms_ > config::timeout_dim_ms)
    {
        return transitionToDim();
    }

    return State::brewing;
}

[[nodiscard]] State dim()
{
    if (button_.is_pressed())
    {
        debug("should tare");
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const float weight_in_grams = hysteresis_.compute(bias_compensated_);

    weight_display_.setSegments(Formatter::to_segments(weight_in_grams).get());
    timer_display_.update();

    // todo maybe do this at the beginning?

    if (!filter_.hasSteadyState())
    {
        return transitionToBrewing();
    }

#ifndef LOGGING  // Don't sleep when logging
    if (millis() - last_activity_time_ms_ > config::timeout_sleep_ms)
    {
        return transitionToSleep();
    }
#endif

    return State::dim;
}

[[nodiscard]] State sleep()
{
    debug("sleep");

    weight_display_.clear();
    timer_display_.stop();
    filter_.clear();
    load_cell_.power_down();

    // Turn off red indicator LED that I stupidly tied the scale display's DIO pin to...
    digitalWrite(13, LOW);

    // When sleep mode is triggered by a long press, we must disable the
    // interrupt for a moment, so that letting go of the button doesn't trigger
    // the ISR and wake the device.
    detachTareButtonInterrupt();
    delay(1000ul);
    attachTareButtonInterrupt();

    sleep_mode();

    load_cell_.power_up();

    return transitionToTaring();
}

//--- SETUP/LOOP ---------------------------------------------------------------

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
    load_cell_.set_scale(config::division_factor);

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

    state_ = transitionToTaring();
}

void loop()
{
    // debug("loop");

    if (button_.is_long_pressed())
    {
        state_ = transitionToSleep();
    }

    switch (state_)
    {
    case State::taring:
        state_ = taring();
        break;

    case State::ready:
        state_ = ready();
        break;

    case State::brewing:
        state_ = brewing();
        break;

    case State::dim:
        state_ = dim();
        break;

    case State::sleep:
        state_ = sleep();
        break;
    }
}
