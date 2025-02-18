# RGB Matrix Basestation

This is an application that runs a 64x64 RGB Matrix display to show an analog clock and weather forecast display.
The weather is pulled from Weather Canada (weather.gc.ca) and is currently set up to track Kentville, Nova Scotia. This can be changed in the weather-station-module.hpp file.

## Getting Started

### Dependencies

- Linux OS [lightweight preferred]
- GCC Compiler
- libcurl

### Installing

1. Follow the detailed instructions on hzeller's README to get your RGB matrix set up and functioning.
2. Clone this repository.
3. Install libcurl (if it's not already)
    Install with: `sudo apt install libcurl4-openssl-dev`
4. Run Make in the RGBMatrixBaseStation root directory.

NOTE: I also designed a 3D printed casing and stand for this basestation. If you are interested in using it for your matrix, please check it out here: https://www.thingiverse.com/thing:6687509

### Executing

Simply run `sudo ./matrix-app` in the RGBMatrixBaseStation/basestation directory.  
