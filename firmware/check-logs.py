#!/usr/bin/env python3
"""Replay logs through the firmware code and check the scale's behaviour.

Fails if a result is worse than in logs/baseline.json. Lower is better for
all results.
"""

import argparse
import csv
import glob
import json
import os
import statistics
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPLAY = os.path.join(HERE, ".pio/build/replay/program")
BASELINE = os.path.join(HERE, "logs/baseline.json")

JUMP_G = 0.25  # A plateau ends when the reading moves this far from it
MIN_PLATEAU_MS = 7000
SETTLE_MS = 4000  # Not checked at the start of a plateau, e.g. after a swirl
TRUTH_MS = 2000  # The true weight is the mean over this time after settling
END_MARGIN_MS = 1000  # Not checked at the end, where the next step starts
ROUNDING_MARGIN_G = 0.02  # Near a rounding boundary, both neighbours pass
LOCAL_MS = 1000  # The current weight is the mean over +-this time


def read_rows(path):
    """Returns the replay output joined with the logged timing and presses."""
    result = subprocess.run([REPLAY, path], capture_output=True, text=True)
    if result.returncode not in (0, 1):
        sys.exit(f"{path}: {result.stderr}")
    with open(path) as file:
        lines = file.read().splitlines()
    header = next(i for i, line in enumerate(lines) if line.startswith("time_ms"))
    logged = csv.DictReader(lines[header:])
    replayed = csv.DictReader(result.stdout.splitlines())
    rows = []
    for rep, log in zip(replayed, logged):
        rows.append({
            "time": int(rep["time_ms"]),
            # The display updates when the sample arrives
            "display_time": int(rep["time_ms"]) + int(log["wait_us"] or 0) // 1000,
            "state": rep["state"],
            "scaled": float(rep["scaled"]) if rep["scaled"] else None,
            "displayed": float(rep["displayed"]) if rep["displayed"] else None,
            "press_ms": int(log["press_ms"]) if log["press_ms"] else None,
        })
    return rows, result.returncode == 1


def find_plateaus(rows):
    """Splits the samples into runs where the sensor reading stays put.

    A tare press also starts a new run.
    """
    runs, run, pressed = [], [], False
    for row in rows:
        pressed = pressed or row["press_ms"] is not None
        if row["scaled"] is None:
            continue
        if run:
            ref = statistics.median(r["scaled"] for r in run[-10:])
            if abs(row["scaled"] - ref) > JUMP_G or pressed:
                runs.append(run)
                run = []
        pressed = False
        run.append(row)
    runs.append(run)
    return [r for r in runs if r[-1]["time"] - r[0]["time"] >= MIN_PLATEAU_MS]


def check_plateau(run, follow, ref):
    """Checks that the display shows the weight at the start of the plateau.

    A plateau near zero must show 0.0, since any reading there is the scale's
    own drift. Other plateaus are measured from ref, the reading at the end of
    the previous zero plateau. If follow is true, the display may also follow
    slow changes, such as evaporation during a brew, and show anything between
    the weight at the start and the current weight.
    """
    start, end = run[0]["time"], run[-1]["time"]
    shown = [r for r in run if r["displayed"] is not None
             and r["time"] <= end - END_MARGIN_MS]
    window = [r for r in shown if r["time"] >= start + SETTLE_MS]
    weight = statistics.mean(r["scaled"] for r in window
                             if r["time"] < start + SETTLE_MS + TRUTH_MS)
    zero = abs(weight) < 0.5
    truth = 0.0 if zero else weight - ref
    values = {r["displayed"] for r in window}

    def correct(row):
        low = high = truth
        if follow and not zero:
            current = statistics.mean(
                r["scaled"] for r in run
                if abs(r["time"] - row["time"]) <= LOCAL_MS) - ref
            low, high = min(truth, current), max(truth, current)
        return (round(low - ROUNDING_MARGIN_G, 1) <= row["displayed"]
                <= round(high + ROUNDING_MARGIN_G, 1))

    final = shown[-1]["displayed"]
    settled_at = start
    for prev, row in zip(shown, shown[1:]):
        if prev["displayed"] != final:
            settled_at = row["time"]

    return {
        "zero": zero,
        "end": statistics.mean(r["scaled"] for r in shown
                               if r["time"] >= end - END_MARGIN_MS - TRUTH_MS),
        "correct": all(correct(r) for r in window),
        "changed": not follow and len(values) > 1,
        "settle_ms": settled_at - start,
    }


def tare_latencies(rows):
    """Time from each tare press to the display update after taring."""
    latencies = []
    for i, row in enumerate(rows):
        # The interrupt can fire during setup. Ignore presses from before the
        # first loop iteration.
        if row["press_ms"] is None or row["press_ms"] < rows[0]["time"]:
            continue
        tared = next(j for j in range(i, len(rows)) if rows[j]["state"] != "0")
        shown = next(r for r in rows[tared:] if r["displayed"] is not None)
        latencies.append(shown["display_time"] - row["press_ms"])
    return latencies


def timer_delays(rows):
    """Time from the reading passing 1 g to the timer starting from ready."""
    delays = []
    for i in range(1, len(rows)):
        if rows[i]["state"] != "2" or rows[i - 1]["state"] != "1":
            continue
        below = max(k for k in range(i + 1)
                    if rows[k]["scaled"] is not None and rows[k]["scaled"] <= 1)
        above = next(r for r in rows[below + 1:] if r["scaled"] is not None)
        delays.append(rows[i]["display_time"] - above["display_time"])
    return delays


def check_log(path):
    rows, mismatched = read_rows(path)
    # Brews evaporate. The other logs use a sealed jar.
    follow = os.path.basename(path).startswith("brew")
    plateaus, ref = [], 0.0
    for run in find_plateaus(rows):
        plateaus.append(check_plateau(run, follow, ref))
        if plateaus[-1]["zero"]:
            ref = plateaus[-1]["end"]
    zero = [p for p in plateaus if p["zero"]]
    loaded = [p for p in plateaus if not p["zero"]]
    latencies = tare_latencies(rows)
    delays = timer_delays(rows)
    results = {
        "wrong_zero": sum(not p["correct"] for p in zero),
        "wrong_loaded": sum(not p["correct"] for p in loaded),
        "changed": sum(p["changed"] for p in plateaus),
        "max_settle_ms": max((p["settle_ms"] for p in plateaus), default=0),
        "max_tare_ms": max(latencies, default=0),
        "max_timer_ms": max(delays, default=0),
    }
    counts = (f"{len(zero)} zero and {len(loaded)} loaded plateaus, "
              f"{len(latencies)} tares, {len(delays)} timer starts")
    return results, counts, mismatched


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("logs", nargs="*")
    parser.add_argument("--update-baseline", action="store_true")
    args = parser.parse_args()
    paths = args.logs or sorted(glob.glob(os.path.join(HERE, "logs/*.csv")))

    try:
        with open(BASELINE) as file:
            baseline = json.load(file)
    except FileNotFoundError:
        baseline = {}

    failed = False
    for path in paths:
        name = os.path.basename(path)
        results, counts, mismatched = check_log(path)
        print(f"{name}: {counts}")
        if mismatched:
            print("  replay differs from the device (expected if the logic changed)")
        for key, value in results.items():
            old = baseline.get(name, {}).get(key)
            if old is None:
                note = "(no baseline)"
            elif value > old:
                note = f"WORSE (baseline {old})"
                failed = True
            elif value < old:
                note = f"better (baseline {old})"
            else:
                note = ""
            print(f"  {key:14} {value:6} {note}")
        baseline[name] = results

    if args.update_baseline:
        with open(BASELINE, "w") as file:
            json.dump(baseline, file, indent=2, sort_keys=True)
            file.write("\n")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
