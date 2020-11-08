# coffee-scales

[![PlatformIO CI](https://github.com/mortenfyhn/coffee-scales/workflows/PlatformIO%20CI/badge.svg)](https://github.com/mortenfyhn/coffee-scales/actions?query=workflow%3A%22PlatformIO+CI%22)

Kitchen scales, built for drip coffee brewing.

* 0.1 gram accuracy.
* Very quick response.
* Auto-starting brew timer.

Current prototype looks something like this:

![A breadboard prototype in action](https://i.imgur.com/RPn1dWZ.jpg)

## Build and run

To build and run the code:

* Install PlatformIO Core and shell commands: https://docs.platformio.org/en/latest/core/installation.html
* `pio run --environment nanoatmega328 --target upload`

## Hardware
* A 1 kg straight bar load cell connected through an HX711 load cell amplifier.
* Two 4-digit 7-segment displays driven by TM1637 LED drivers.
* An Arduino Nano, might get replaced by a suitable MCU.
