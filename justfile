set working-directory := 'firmware'

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

# Upload logging firmware, then capture a log until Enter
log: && capture
  pio run --environment logging --target upload

# Capture a log without uploading. Opening the port resets the board.
capture:
  mkdir -p logs/temp
  ./capture-log.py "logs/temp/$(date +"%Y-%m-%dT%H:%M:%S").csv"

# Replay a log on the host and print the processing stages as CSV
replay log *flags:
  @pio run --environment replay --silent
  @cd {{invocation_directory()}} && {{justfile_directory()}}/firmware/.pio/build/replay/program {{flags}} {{log}}

# Run the CI checks
ci:
  {{justfile_directory()}}/.semaphore/ci.sh
