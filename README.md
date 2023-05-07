# coffee-scales

[![PlatformIO CI](https://github.com/mortenfyhn/coffee-scales/actions/workflows/main.yml/badge.svg)](https://github.com/mortenfyhn/coffee-scales/actions/workflows/main.yml)

Kitchen scales, built for drip coffee brewing:

* 0.1 gram precision
* Very quick response
* Auto-starting brew timer

Current prototype looks something like this:

![A breadboard prototype in action](https://i.imgur.com/RPn1dWZ.jpg)

## Build and run

To build and run the code:

1. Install PlatformIO Core and shell commands: https://docs.platformio.org/en/latest/core/installation.html
2. `pio run -e feather328 -t upload`

More environments are defined in `platformio.ini`.

## Hardware

* Straight bar load cell (1 kg capacity)
* HX711 load cell amplifier
* Two 4-digit 7-segment displays with TM1637 LED drivers
* Any Arduino (currently an Adafruit Feather 328P)
* A small LiPo battery
* A button for taring
