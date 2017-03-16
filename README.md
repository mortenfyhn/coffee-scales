# coffee-scale
A digital kitchen scale, intended for pourover coffee. Work in progress.

## Hardware
* A load cell connected through an HX711 weight scale ADC.
* Two 4-digit 7-segment displays driven by TM1637 LED drivers.
* Probably a few buttons.
* An Arduino Nano, might get replaced by a suitable bare AVR chip.

## Features (planned)
* High precision mode: 1/10 gram accuracy, slower settling time. Used for weighting coffee beans.
* High speed mode: 1 gram accuracy, faster settling time. Used for weighting the brew water.
* Auto timer: Timer starts automatically when you add weight. Resets when tared.
* Tare function: Kind of have to have one.
