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

# Run unit tests
just test

# Build and upload debug firmware, then monitor serial output
just debug

# Upload logging firmware and capture/process logs
just log
```

### Build Environments

PlatformIO defines multiple environments in `firmware/platformio.ini`:
- `release` (default): Production firmware
- `logging`: Enables CSV logging via serial for debugging
- `debug`: Debug build with serial output
- `test`: Native unit tests (runs on host, not on device)

Build specific environment: `just build <env>` (e.g., `just build logging`)

## Architecture

### State Machine

The firmware (`firmware/src/main.cpp`) implements a state machine with these states:

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

The pipeline configuration is defined in `firmware/src/config.h` and must match `firmware/process-log.m` for offline analysis.

### Custom Libraries

Located in `firmware/lib/`, these are project-specific modules:

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

#### Interactive Analysis with Jupyter (Recommended)

For rapid iteration on signal processing algorithms:

1. Run `just log` to capture serial data to CSV (or use existing log files)
2. Start Jupyter: `cd firmware/logs && jupyter notebook`
3. Open `process.ipynb` in your browser
4. Upload CSV file using the file picker widget
5. Adjust processing parameters with interactive sliders:
   - `division_factor`: Load cell calibration
   - `filter_size`: Running average window
   - `hysteresis_size`: Display stabilization threshold
   - `std_threshold`: Steady state detection threshold
6. Re-run cells to see results update in interactive plots

The notebook shows all pipeline stages with interactive Plotly graphs (zoom, pan, hover for values).

**Setup Jupyter** (one-time):
```sh
pip install --user jupyter ipywidgets plotly pandas numpy matplotlib
```

#### Legacy Octave Processing

The `firmware/process-log.m` Octave/MATLAB script provides offline batch processing. It applies the same filtering pipeline and outputs all intermediate stages to CSV for visualization in PlotJuggler.

**Important**: When modifying filter parameters in `firmware/src/config.h`, update matching values in both processing scripts (`process.ipynb` and `process-log.m`):
- `division_factor`: Load cell calibration constant
- `filter_size`: Running average window size
- `hysteresis_size`: Hysteresis threshold (grams)
- `standard_deviation_threshold`: Steady state detection threshold (in `SmoothingFilter.h`)

### Hardware Configuration

Pin assignments in `firmware/src/config.h`:
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
just test
```

Keep `justfile` CI recipe in sync with `.semaphore/ci.sh`.
