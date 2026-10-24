# Test harness plan (temporary, delete when done)

Goal: test firmware behaviour on real log data before changing the behaviour.

## Goals to test

Flaky today, should pass after later fixes:

1. Tare shows 0.0 and stays at 0.0.
2. 0.0 -> add weight -> remove weight shows 0.0.
3. No drift with a fixed weight on the scale (5 min).

Works today, must not get worse:

4. Tare latency stays as today (mean 130 ms, max 170 ms, press to display). Feels instant, no need to be faster.
5. Response speed, settling time and the auto-start timer.
6. Loop finishes before the next HX711 sample (about 86-90 ms).

## Steps

- [x] 1. Logging build logs one row per loop iteration: inputs (time, raw, button), outputs (state, displayed weight) and how long `read()` waited.
- [ ] 2. Capture logs with the 200 g jar: tares, add/remove cycles, 5 min hold (done, hot start). Cold start: using the old empty-scale log (converted to `logs/coldstart.csv`). Optional later: cold-start brew or hold with a load.
- [x] 3. Move pipeline and state machine into code that builds on AVR and on the host. Replay must reproduce the logged state and displayed weight.
- [x] 4. Checks on the goals above, per log.
- [x] 5. Delete `process-log.m`. Host replay prints intermediate stages as CSV for plotting.

## Checks at every step

- `release`, `logging` and `test` build. Flash and RAM within limits.
- Replay matches device output (from step 3).
- Loop timing within budget.

## Baseline (before step 1)

- release: 18210 B flash, 265 B RAM. logging: 21704 B flash, 456 B RAM.
- Current board, current logging build: mean sample interval 89.9 ms (40 s, empty scale).

## After step 1

- release: 18132 B flash, 265 B RAM. logging: 19900 B flash, 523 B RAM.
- 40 s, empty scale: mean sample interval 89.8 ms. `read()` waits about 84 ms, so the loop does about 6 ms of work. The 90 ms interval is this board's HX711 clock, not an overrun.

## After step 3

- release: 18710 B flash, 305 B RAM (+578 B, +40 B, mostly the virtual Hardware interface).
- Replay of the pre-refactor 40 s log: 417 rows, 0 mismatches.
- Refactored logging build on the device: 30 s log replays with 0 mismatches. Interval 90.1 ms, wait about 83.8 ms.

## Open

- Goal 1 (tare hits 0.0) was not reproduced in session 1: 17 tares, all 0.0. The user will watch for when it fails.

## After step 4 (hot logs only)

- Baseline: return-to-zero 5/11 zero plateaus wrong, 5/10 loaded wrong. Hold: removal ends at 0.1. Tares: all correct, max latency 169 ms.
- Sensor returns to zero within 0.014 g. The errors come from bias compensation (keeps its offset after the load is removed) and hysteresis (latches at 0.1).
- Experiment (reverted): without bias compensation, wrong_zero is 0 in all hot logs. Cold-start logs decide whether bias compensation is needed (old coldstart.csv: 0.2 g warm-up drift in 5 min).
- Spurious tare press at boot: the interrupt fires during setup (press_ms 0). Harmless, taring() clears it.
- Cold start (old log, empty scale): today's firmware stays at 0.0 for 10 min. Without bias compensation, the display drifts to 0.3, so bias compensation is needed.
- Brew (cold, about 20 min asleep): evaporation 0.6-0.9 g/min. Bias compensation carries it across pours: display 404.3 g against 400.3 g measured at the end. The user doesn't care whether evaporation is cancelled (very slight preference for not cancelling), only that the scale's own creep is. Check rule for `brew*.csv`: display between the plateau's start weight and the current weight.
- Cold start under load (brew boot tare with 662 g): +0.07 g in the first 15 s.

## Behaviour fix

- Step 1: hysteresis keeps the shown tenth while the input is within 0.05 + 0.02 g, otherwise shows the nearest tenth. The timer start uses the value before rounding.
- Step 2: the bias resets when the filter is not steady, and the jump to the average at steady entry is not counted. The display freezes at the average at steady entry. Hot logs: all plateaus correct.
- Load cell creep after a step: 0.02-0.05 g over several seconds, towards the new value. Freezing at entry is therefore about 0.03 g early. Freezing later reduces loaded errors in cold-steps but slows settling (K=30: 2/12 wrong, 3.4 s). Chosen: freeze at entry. Better hardware makes it more accurate without code changes.
- Open: while frozen, the display only changes on a step of about 0.2 g or a pour faster than about 0.4 g/s. Smaller or slower changes are hidden as creep (same as the original code). Lowering the steady threshold would help with less noisy hardware.
