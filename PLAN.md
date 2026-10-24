# Test harness plan (temporary, delete when done)

Goal: test firmware behaviour on real log data before changing the behaviour.

## Goals to test

Flaky today, should pass after later fixes:

1. Tare shows 0.0 and stays at 0.0.
2. 0.0 -> add weight -> remove weight shows 0.0.
3. No drift with a fixed weight on the scale (5 min).

Works today, must not get worse:

4. Tare takes less than 100 ms when the reading is steady.
5. Response speed, settling time and the auto-start timer.
6. Loop finishes before the next HX711 sample (about 86-90 ms).

## Steps

- [x] 1. Logging build logs one row per loop iteration: inputs (time, raw, button), outputs (state, displayed weight) and how long `read()` waited.
- [ ] 2. Capture logs with the 200 g jar: tares, add/remove cycles, 5 min hold, a real brew.
- [x] 3. Move pipeline and state machine into code that builds on AVR and on the host. Replay must reproduce the logged state and displayed weight.
- [ ] 4. Checks on the goals above, per log.
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

- Old logs in `firmware/logs/*.csv` are in the old format (time in s, raw, processed columns) and have no button events. Convert them or drop them?
- Tare latency (goal 4) needs the press time, which the log doesn't have yet. Add a column after the step 2 captures.
