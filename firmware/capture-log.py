#!/usr/bin/env python3
"""Write serial output from the scale to a file until Enter or --seconds."""

import argparse
import glob
import select
import sys
import time

import serial


def find_port():
    ports = glob.glob("/dev/serial/by-id/*")
    if len(ports) != 1:
        sys.exit(f"Expected one serial device, found {ports}. Use --port.")
    return ports[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output")
    parser.add_argument("--port")
    parser.add_argument("--seconds", type=float)
    args = parser.parse_args()

    # Opening the port resets the board, so the log starts at boot.
    port = serial.Serial(args.port or find_port(), 38400, timeout=1)
    end = time.monotonic() + args.seconds if args.seconds else None
    inputs = [port, sys.stdin] if sys.stdin.isatty() else [port]
    if sys.stdin.isatty():
        print("Capturing. Press Enter to stop.", file=sys.stderr)
    with open(args.output, "wb") as output:
        try:
            while end is None or time.monotonic() < end:
                ready, _, _ = select.select(inputs, [], [], 1)
                if sys.stdin in ready:
                    sys.stdin.readline()
                    break
                if port in ready:
                    line = port.readline()
                    output.write(line)
                    output.flush()
                    sys.stdout.buffer.write(line)
                    sys.stdout.flush()
        except KeyboardInterrupt:
            pass
    print(f"Saved to {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
