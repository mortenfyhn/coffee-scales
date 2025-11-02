set working-directory := 'firmware'

export PATH := env('PATH') + ':' + env('HOME') + '/.platformio/penv/bin'

# Build but don't upload
build env="release":
  pio run --environment {{env}}

# Build and upload to a board
upload env="release":
  pio run --environment {{env}} --target upload

# Run unit tests
test:
  pio test --environment test

# Build and upload for debug, and monitor serial output
debug:
  pio run --environment debug --target upload
  pio device monitor

# Upload logging firmware, then capture and process a log
log:
  #!/usr/bin/env bash
  set -euo pipefail
  pio run --environment logging --target upload
  mkdir -p logs/temp
  timestamp="$(date +"%Y-%m-%dT%H:%M:%S")"
  raw_file="logs/temp/${timestamp}.csv"
  processed_file="logs/temp/${timestamp}-processed.csv"
  echo "Starting serial monitor..."
  pio device monitor --quiet | tee "$raw_file"
  echo "Raw log saved to $raw_file"
  echo "Processing..."
  ./process-log.m "$raw_file" > "$processed_file"
  echo "Processed log saved to $processed_file"

# Run the CI checks
# NOTE: Sync this manually with .semaphore/ci.sh
ci:
  just build
  just build logging
  just test
