# coffee-scales

[![Build Status](https://fyhn.semaphoreci.com/badges/coffee-scales/branches/master.svg?key=bdad7781-738b-46c5-95e4-85f3478b0b54)](https://fyhn.semaphoreci.com/projects/coffee-scales)

Kitchen scales, built for drip coffee brewing:

* 0.1 gram precision
* Very quick response
* Very quick tare
* Auto-starting brew timer
* Auto-off after 5 minutes

Current prototype looks something like this:

![A breadboard prototype in action](https://i.imgur.com/sJAW9rS.jpg)

## Build

To build and upload the code:

1. [Install PlatformIO Core and shell commands](https://docs.platformio.org/en/latest/core/installation/index.html)
2. `pio run -t upload`

## Log data

Run

```sh
pio run -e logging -t upload && pio device monitor --quiet | tee logs/log.csv
```

This will log to your screen and to a file `logs/log.csv` 🪵

## Hardware

* Adafruit Feather 328P
* Load cell: 1 kg straight bar
* HX711 load cell amplifier
* Two 4-digit 7-segment displays with TM1637 drivers
* LiPo battery (1200 mAh)
* Push button for taring

![Connection diagram](https://i.imgur.com/zfTGbLD.png)
