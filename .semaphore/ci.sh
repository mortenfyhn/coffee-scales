#!/usr/bin/env bash

# * [ ] use just

pio run --environment release
pio run --environment logging
pio test --environment test
