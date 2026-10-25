#include <Scale.h>
#include <config.h>
#include <math.h>

Scale::Scale(Hardware& hardware, bool auto_sleep)
    : hardware_(hardware), auto_sleep_(auto_sleep),
      filter_(config::filter_size, config::steady_threshold), hysteresis_(config::hysteresis_size)
{
}

void Scale::start()
{
    state_ = transitionToTaring();
}

void Scale::step()
{
    if (hardware_.long_pressed())
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

Stages Scale::stages() const
{
    return {scaled_, prev_filtered_, filter_.hasSteadyState(), bias_,
            bias_compensated_};
}

void Scale::read_load_cell_and_update_filter()
{
    const int32_t raw_value = hardware_.read_load_cell();
    const float weight_in_grams_raw =
        (raw_value - offset_) / config::division_factor;
    scaled_ = weight_in_grams_raw;
    filter_.addValue(weight_in_grams_raw);

    const auto curr_filtered = filter_.getValue();

    const auto is_steady = filter_.hasSteadyState();

    if (was_steady_ && is_steady)
    {
        bias_ += curr_filtered - prev_filtered_;
    }
    else if (!is_steady)
    {
        bias_ = 0.f;
    }

    const auto became_steady = !was_steady_ && is_steady;
    was_steady_ = is_steady;

    if (became_steady)
    {
        if (fabsf(curr_filtered) < config::zero_tracking_band)
        {
            tare();
            return;
        }
    }

    bias_compensated_ = curr_filtered - bias_;
    prev_filtered_ = curr_filtered;
}

void Scale::tare()
{
    const int32_t new_offset =
        offset_ + filter_.getValue() * config::division_factor;
    offset_ = new_offset;
    hysteresis_.reset();

    bias_ = 0.f;
    prev_filtered_ = 0.f;
    bias_compensated_ = 0.f;
}

//--- TRANSITIONS --------------------------------------------------------------

State Scale::transitionToTaring()
{
    hardware_.set_brightness_max();
    hardware_.stop_timer();
    last_activity_time_ms_ = hardware_.millis();

    return State::taring;
}

State Scale::transitionToReady()
{
    hardware_.set_brightness_max();

    return State::ready;
}

State Scale::transitionToBrewing()
{
    hardware_.set_brightness_max();
    hardware_.start_timer();

    return State::brewing;
}

State Scale::transitionToDim()
{
    hardware_.set_brightness_dim();

    return State::dim;
}

State Scale::transitionToSleep()
{
    return State::sleep;
}

//--- STATES -------------------------------------------------------------------

State Scale::taring()
{
    // I do this to clear any pending "should tares".
    // TODO: Figure out if needed, if not remove, if yes refactor.
    hardware_.tare_pressed();

    // Placing this check early makes taring instant when the filter is already
    // stable, without wasting time on showing the "wait for taring" line.
    if (filter_.hasSteadyState())
    {
        tare();
        return transitionToReady();
    }

    hardware_.show_line();

    read_load_cell_and_update_filter();

    return State::taring;
}

State Scale::ready()
{
    if (hardware_.tare_pressed())
    {
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const float weight_in_grams = hysteresis_.compute(bias_compensated_);
    hardware_.show_weight(weight_in_grams);

    if (!filter_.hasSteadyState())
    {
        last_activity_time_ms_ = hardware_.millis();
    }

    if (bias_compensated_ > 1.f)
    {
        return transitionToBrewing();
    }

    if (hardware_.millis() - last_activity_time_ms_ > config::timeout_dim_ms)
    {
        return transitionToDim();
    }

    return State::ready;
}

State Scale::brewing()
{
    if (hardware_.tare_pressed())
    {
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const float weight_in_grams = hysteresis_.compute(bias_compensated_);
    hardware_.show_weight(weight_in_grams);
    hardware_.update_timer();

    if (!filter_.hasSteadyState())
    {
        last_activity_time_ms_ = hardware_.millis();
    }

    if (hardware_.millis() - last_activity_time_ms_ > config::timeout_dim_ms)
    {
        return transitionToDim();
    }

    return State::brewing;
}

State Scale::dim()
{
    if (hardware_.tare_pressed())
    {
        return transitionToTaring();
    }

    read_load_cell_and_update_filter();

    const float weight_in_grams = hysteresis_.compute(bias_compensated_);

    hardware_.show_weight(weight_in_grams);
    hardware_.update_timer();

    // todo maybe do this at the beginning?

    if (!filter_.hasSteadyState())
    {
        return transitionToBrewing();
    }

    if (auto_sleep_ &&
        hardware_.millis() - last_activity_time_ms_ > config::timeout_sleep_ms)
    {
        return transitionToSleep();
    }

    return State::dim;
}

State Scale::sleep()
{
    filter_.clear();
    hardware_.sleep();

    return transitionToTaring();
}
