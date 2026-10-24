#pragma once

#include <Hysteresis.h>
#include <SmoothingFilter.h>
#include <stdint.h>

enum class State : uint8_t
{
    taring,
    ready,
    brewing,
    dim,
    sleep,
};

// What the scale logic needs from the device. The firmware implements this with
// the real hardware. The host replay implements it with values from a log.
class Hardware
{
  public:
    virtual uint32_t millis() = 0;
    virtual int32_t read_load_cell() = 0;  // Waits for the next sample
    virtual bool tare_pressed() = 0;
    virtual bool long_pressed() = 0;
    virtual void show_line() = 0;
    virtual void show_weight(float grams) = 0;
    virtual void set_brightness_max() = 0;
    virtual void set_brightness_dim() = 0;
    virtual void start_timer() = 0;
    virtual void stop_timer() = 0;
    virtual void update_timer() = 0;
    // Clears the displays, powers down and waits for the tare button.
    virtual void sleep() = 0;

  protected:
    ~Hardware() = default;
};

// Intermediate values of the signal processing, for offline analysis.
struct Stages
{
    float scaled;
    float filtered;
    bool steady;
    float bias;
    float compensated;
};

// The state machine and signal processing. Runs on the device and on the host.
class Scale
{
  public:
    // auto_sleep: sleep after config::timeout_sleep_ms without activity.
    Scale(Hardware& hardware, bool auto_sleep);

    void start();
    void step();  // One loop iteration

    State state() const { return state_; }
    Stages stages() const;

  private:
    void read_load_cell_and_update_filter();
    void tare();

    [[nodiscard]] State transitionToTaring();
    [[nodiscard]] State transitionToReady();
    [[nodiscard]] State transitionToBrewing();
    [[nodiscard]] State transitionToDim();
    [[nodiscard]] State transitionToSleep();

    [[nodiscard]] State taring();
    [[nodiscard]] State ready();
    [[nodiscard]] State brewing();
    [[nodiscard]] State dim();
    [[nodiscard]] State sleep();

    Hardware& hardware_;
    const bool auto_sleep_;
    State state_ = State::taring;
    SmoothingFilter filter_;
    Hysteresis hysteresis_;
    int32_t offset_ = 0;
    uint32_t last_activity_time_ms_ = 0;
    float scaled_ = 0.f;
    float bias_ = 0.f;
    float prev_filtered_ = 0.f;
    float bias_compensated_ = 0.f;
};
