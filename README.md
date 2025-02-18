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



## Further TODO

### 1. Create a "Daytime Mode" for the Weather Module
Make a daytime mode for the weather station module.

During the time between sunrise and sunset, adopt a special all-white colour scheme for the display to increase visibility.

Make all text and things that are currently gray or other colours, white.
As for the icons, you can keep them the way they are, OR create a special set of icons that are only a couple of the more visible colours and are easy to see. I.e. Make clouds solid white, etc.

### 2. Weather Module Updates
- Consider parallelizing the data retrieval from MSC Datamart so as to not interrupt frame updating too much. Learn about C/C++ multithreading to determine the best way forward with this.
- Add a feature to the weather Module that puts a little blue dot next to the days that are nice enough to ride a motorcycle. (Add a bool flag to the WeatherDay struct to say yes or no). Create a function that analyzes the temperature high and full forecast text conditions for signs that it would be a good day to ride. If the temperature is below a certain level, then no blue dot. If the text says rain, etc., anywhere then no blue dot.

### 3. Initial Loading Screen
Add a loading screen at powerup and animation to show that it's powering up.
Wait for a valid connection to the internet to be established so that the clock is correct. Show a loading/startup screen while this occurs. Then, once established, animate an opening to the clock module.
Make sure that there is an internet connection, and force update the date and time on Linux before constructing the modules. (Do a loading screen while this happens).

To force update the time on linux, make sure that an internet connection is established and run:
'timedatectl set-ntp true'

### 4. Refactor
Fix the code structure. Particularly the file structure. Everything is just in "my-project" folder right now.

### 5. Module to Module Transition Animation
Figure out how to do animations from module to module (like sliding one module off the screen while simultaneously sliding another module onto the screen).

Questions:
- What happens if you try to draw a canvas that is too big for the matrix? like a 128*64 canvas on a 64*64 display?
- Can you draw a canvas partially off the display?
- How do they implement the scrolling text example?? Maybe get some insights from this...?

The two options that stand out to me now are:
1. Instead of drawing directly to FrameCanvas and having multiple of them, instead draw each canvas as seperate byte arrays (or similar), then have a transition function that manipulates both fully drawn prepped "canvases" (byte arrays, or equiv) to combine them properly, and THEN write the final buffer to a single FrameCanvas.
2. Continue to explore the Canvas and FrameCanvas types to see if there is someway that they can be manipulated. What do the Serialize/Deserialize methods do? Could I get a useable value from the Serializaion of each canvas, combine them appropriately, and then deserialize to create the final canvas?

Potentially useful link:
https://github.com/hzeller/rpi-rgb-led-matrix/issues/408

### 6. Add an Easy Shutdown Method
Add a button or some simple trigger to cause the RaspberryPi to issue a safe shutdown command.
Perhaps have it play a power down animation on the matrix display.
See if you can get it to display something on the RGB matrix while it's shutting down and then only when it's fully shutdown will it turn off so that it's clear to the user that it's safe to unplug.
If not, then just have it catch the shutdown command in the app, play a quick animation, and then let it do it's thing.

Or maybe! Make the SD card read only so that there's no chance of corrupting the card when pulling the plug!!
Have the boot script update the file, and make the matrix-app program, but then set it to read only before running the program??

### 7. Fix Power Supply Whine
Try to fix the coil whine issue on one of the power supplies.
- Open it.
- Check which coils are causing whine.
- Use hot glue (or some type of heat resistant resin) to prevent the vibration.
- Test it before putting it back together.

### 8. Add Small Fan for CPU Active Cooling
This is not strictly necessary right now as it seems to remain at reasonable temps without any active cooling, but it may be nice for the future. It depends on how hot the CPU gets.
