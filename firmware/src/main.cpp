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

// TODO maybe a "wait to stabilize" state

enum class State
{
    taring,
    ready,
    active,
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
auto last_activity_time_ms_ = 0ul;

float read_battery_voltage()
{
    return config::battery_scaling * analogRead(pins::battery_voltage);
}

void read_load_cell_and_update_filter()
{
    const auto raw_value = load_cell_.read();
    log(raw_value);
    const auto weight_in_grams_raw =
        (raw_value - load_cell_.get_offset()) / config::scale_factor;
    filter_.addValue(weight_in_grams_raw);
}

void tare()
{
    const auto new_offset =
        load_cell_.get_offset() + filter_.getValue() * config::scale_factor;
    load_cell_.set_offset(new_offset);
    hysteresis_.reset();
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

    timer_display_.stop();
    last_activity_time_ms_ = millis();

    return State::taring;
}

[[nodiscard]] State transitionToReady()
{
    debug(" -> ready");

    weight_display_.setMaxBrightness();
    timer_display_.setMaxBrightness();

    return State::ready;
}

[[nodiscard]] State transitionToActive()
{
    debug("  -> active");

    weight_display_.setMaxBrightness();
    timer_display_.setMaxBrightness();
    timer_display_.start();

    return State::active;
}

[[nodiscard]] State transitionToDim()
{
    debug("   -> dim");

    weight_display_.setMinBrightness();
    timer_display_.setMinBrightness();

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

    const auto weight_in_grams = hysteresis_.compute(filter_.getValue());
    weight_display_.setSegments(Formatter::to_segments(weight_in_grams).get());

    if (!filter_.hasSteadyState())
    {
        last_activity_time_ms_ = millis();
    }

    if (weight_in_grams > 1.f)
    {
        return transitionToActive();
    }

    if (millis() - last_activity_time_ms_ > config::timeout_dim_ms)
    {
        return transitionToDim();
    }

    return State::ready;
}

[[nodiscard]] State active()
{
    if (button_.is_pressed())
    {
        debug("should tare");
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const auto weight_in_grams = hysteresis_.compute(filter_.getValue());
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

    return State::active;
}

[[nodiscard]] State dim()
{
    if (button_.is_pressed())
    {
        debug("should tare");
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const auto weight_in_grams = hysteresis_.compute(filter_.getValue());

    weight_display_.setSegments(Formatter::to_segments(weight_in_grams).get());
    timer_display_.update();

    // todo maybe do this at the beginning?

    if (!filter_.hasSteadyState())
    {
        return transitionToActive();
    }

    if (millis() - last_activity_time_ms_ > config::timeout_sleep_ms)
    {
        return transitionToSleep();
    }

    return State::dim;
}

[[nodiscard]] State sleep()
{
    debug("sleep");

    weight_display_.clear();
    timer_display_.stop();
    filter_.clear();
    load_cell_.power_down();

    // When sleep mode is triggered by a long press, we must disable the
    // interrupt for a moment, so that letting go of the button doesn't trigger
    // the ISR and wake the device.
    detachTareButtonInterrupt();
    delay(1000ul);
    attachTareButtonInterrupt();

    sleep_mode();

    load_cell_.power_up();
    weight_display_.setMaxBrightness();
    timer_display_.setMaxBrightness();

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
    load_cell_.set_scale(config::scale_factor);

    weight_display_.showNumberDecEx(read_battery_voltage() * 1000, 0b10000000);
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

    case State::active:
        state_ = active();
        break;

    case State::dim:
        state_ = dim();
        break;

    case State::sleep:
        state_ = sleep();
        break;
    }
}
