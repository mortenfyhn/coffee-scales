# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an embedded firmware project for custom coffee scales designed for drip coffee brewing. The scales feature:
- 0.1 gram precision with quick response
- Auto-starting brew timer
- Auto-off after 5 minutes
- State machine-based control (taring, ready, brewing, dim, sleep)

## Build System

The project uses **PlatformIO** for firmware builds and **Just** as a command runner. All commands should be run from the repository root.

### Essential Commands

```sh
# Build firmware (release configuration)
just build

# Build and upload to connected board
just upload

# Run unit tests, and check the scale's behaviour on the logs
just test

# Build and upload debug firmware, then monitor serial output
just debug

# Upload logging firmware and capture a log until Enter
just log

# Capture a log without uploading (resets the board)
just capture

# Replay a log on the host and print the processing stages as CSV
just replay firmware/logs/temp/<log>.csv
```

### Build Environments

PlatformIO defines multiple environments in `firmware/platformio.ini`:
- `release` (default): Production firmware
- `logging`: Enables CSV logging via serial for debugging
- `debug`: Debug build with serial output
- `test`: Native unit tests (runs on host, not on device)
- `replay`: Host program that replays logs through the scale logic

Build specific environment: `just build <env>` (e.g., `just build logging`)

## Architecture

### State Machine

The state machine and signal processing are in `firmware/lib/Scale`. They use the device only through the `Hardware` interface in `Scale.h`. `firmware/src/main.cpp` implements `Hardware` with the real devices, and `firmware/lib/Replay` implements it with values from a log. The state machine has these states:

1. **taring**: Waits for stable reading, then zeros the scale
2. **ready**: Shows weight, waiting for brewing to start (weight > 1g triggers brewing)
3. **brewing**: Shows weight and active timer
4. **dim**: Reduced brightness after 1 min of inactivity
5. **sleep**: Powers down after 5 min (wake on tare button)

State transitions are driven by:
- Button presses (tare button on pin 2, interrupt-driven)
- Weight changes (auto-start brewing when weight exceeds threshold)
- Timeouts (dim after 1 min, sleep after 5 min)

### Signal Processing Pipeline

Load cell readings go through a multi-stage filtering pipeline to achieve responsive yet stable measurements:

1. **Scaling**: Raw HX711 value → grams using division factor (calibrated constant)
2. **Smoothing**: `SmoothingFilter` applies running average when steady state detected (std dev < 0.1), otherwise passes through unfiltered value for responsiveness
3. **Bias compensation**: Compensates for drift by tracking cumulative changes during steady state periods
4. **Hysteresis**: Final stage prevents display flicker (0.1g threshold)

The pipeline configuration is defined in `firmware/lib/Config/config.h`. The host replay uses the same code and configuration as the firmware.

### Custom Libraries

Located in `firmware/lib/`, these are project-specific modules:

- **Scale**: State machine and signal processing, shared by the firmware and the host replay
- **Replay**: Reads logs and runs `Scale` with their inputs (host only)
- **Config**: Constants for the signal processing and timeouts
- **LogRow**: The row format of the logging build
- **SmoothingFilter**: Adaptive filter using RunningAverage; switches between filtered (steady) and raw (changing) values based on standard deviation
- **Hysteresis**: Prevents output changes unless input crosses threshold
- **Display/TimerDisplay**: TM1637 7-segment display drivers
- **Formatter**: Converts float weights to 4-digit display segments
- **Buffer**: Lightweight fixed-size buffer template

External dependencies (managed by PlatformIO):
- HX711: Load cell amplifier driver
- RunningAverage: Moving average implementation
- SevenSegmentTM1637: Base display library (custom fork)

## Development Workflow

### Debugging and Logging

The `logging` build prints one CSV row per loop iteration with the loop's inputs and outputs (see `firmware/lib/LogRow/log_row.h`):

- `time_ms`: `millis()` at the start of the iteration
- `raw`: raw HX711 value, empty if the iteration read no sample
- `wait_us`: how long the read waited for the HX711. The loop's work time is the sample interval minus this.
- `pressed`, `long_pressed`: button events the logic consumed in this iteration
- `state`: state after the iteration (0 taring, 1 ready, 2 brewing, 3 dim, 4 sleep)
- `weight_x10`: displayed weight in tenths of a gram, empty if not shown
- `press_ms`: time of the tare button press, if `pressed` is 1

The logging build does not auto-sleep.

`just log` uploads the logging build and captures to `firmware/logs/temp/` with `firmware/capture-log.py`. Opening the port resets the board, so a log starts at boot.

`just replay LOG` runs the log's inputs through `lib/Scale` on the host. It prints the processing stages as CSV for plotting, for example in PlotJuggler. It also reports iterations where the replayed state or displayed weight differ from the log, and exits with status 1 if there are any. Pass `--auto-sleep` to replay the release behaviour.

### Checks on logs

`firmware/check-logs.py` replays each log in `firmware/logs/` and checks the behaviour. It is part of `just test`. Plateaus are runs of at least 7 s where the sensor reading stays within 0.25 g. The first 4 s of a plateau are not checked, for example the sloshing after a swirl. The true weight of a plateau is the mean of the scaled reading 4-6 s after it starts. Plateaus under 0.5 g count as zero.

Logs with a sealed jar or an empty scale: any slow change is the scale's own creep, so the display must show the true weight and must not change. Logs named `brew*.csv`: brews evaporate, so the display may follow slow changes or hide them. It must stay between the true weight and the current weight.

Results, lower is better:

- `wrong_zero`, `wrong_loaded`: plateaus where the display is wrong by the rules above
- `changed`: plateaus where the display changes (not for brews)
- `max_settle_ms`: time from a plateau's start until the display shows its final value
- `max_tare_ms`: time from a tare press until the display updates
- `max_timer_ms`: time from the reading passing 1 g until the timer starts

The check fails if a result is worse than in `firmware/logs/baseline.json`. After an improvement, or after adding a log, run `./check-logs.py --update-baseline` and commit the new baseline.

### Hardware Configuration

Pin assignments in `firmware/src/pins.h`:
- Load cell (HX711): SCL (DT), SDA (SCK)
- Displays: Two TM1637 modules for weight and timer
- Tare button: Pin 2 (must be interrupt-capable)
- Battery voltage sensing: A6 (analog)

Target hardware: Adafruit Feather 328P with 1kg load cell

### CI

CI runs via Semaphore (`.semaphore/ci.sh`):
```sh
just build
just build logging
just build replay
just test
```

Keep `justfile` CI recipe in sync with `.semaphore/ci.sh`.
