set working-directory := 'firmware'

export PATH := env_var('PATH') + ':' + env_var('HOME') + '/.platformio/penv/bin'

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

# Build and upload for logging, and capture log
log:
  pio run --environment logging --target upload
  pio device monitor --quiet | tee logs/log.csv

# Run the CI checks
# NOTE: Sync this manually with .semaphore/ci.sh
ci:
  just build
  just build logging
  just test
