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

# Upload logging firmware, then capture a log until Ctrl-C
log:
  pio run --environment logging --target upload
  mkdir -p logs/temp
  ./capture-log.py "logs/temp/$(date +"%Y-%m-%dT%H:%M:%S").csv"

# Run the CI checks
ci:
  {{justfile_directory()}}/.semaphore/ci.sh
