// TODO:  I'm not sure how my program will support animations like
//        sliding where potentially 2 MatrixModules will be displayed
//        simultaneously for a few seconds as one slides to replace the first...
//        Doing this may require a redesign...
// TODO: Clean up all these includes. I don't think I need even half of what's here.
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <algorithm>
#include <iostream>  // Need (Used for debugging)
#include <cmath>  // Need [for angle calculations]
#include <ctime>  // Need [for clock]
#include <numbers>
#include <sstream>    // Need [for string manipulation]
#include <stdexcept>  // Need [for throwing exceptions]
#include <string>     // Need [for strings]

//#include "graphics.h"
//#include "led-matrix.h"
//#include "pixel-mapper.h"
#include "matrix-module.hpp"
#include "clock-module.hpp"
#include "weather-station-module.hpp"

const int REFRESH_RATE = 90;
const int SLEEP = 1000000/80;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

// ~~~ BEGIN FUNCTION DEFINITIONS ~~~ //
static int usage(const char* progname) {
	fprintf(stderr, "usage: %s <options>\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t<No Options Set Yet>              : N/A\n");

	rgb_matrix::PrintMatrixFlags(stderr);
	return 1;
}

int main(int argc, char* argv[]) {
	using namespace rgb_matrix;

	RGBMatrix::Options matrix_options;
	rgb_matrix::RuntimeOptions runtime_opt;

	// These are the defaults when no command-line flags are given.
	matrix_options.hardware_mapping = "adafruit-hat-pwm";
	runtime_opt.gpio_slowdown = 1;  // Default value (works well)
	matrix_options.rows = 64;
	matrix_options.cols = 64;
	matrix_options.pixel_mapper_config = "rotate:90";
	matrix_options.chain_length = 1;
	matrix_options.parallel = 1;

	// Cut down on weird graphical glitches:
	//    If left uncapped, graphical glitches occur. but if capped to a constant
	//    rate, they dissapear. Also worth noting that if decreasing this value
	//    too much, the brightness of the leds decreases.
	matrix_options.limit_refresh_rate_hz = REFRESH_RATE;
	//    Increasing this value should increase the on-time of the pixels which,
	//    theoretically (untested) would help increase the brightness when
	//    limiting the refresh rate to lower values.
	matrix_options.pwm_lsb_nanoseconds = 130;  // Default value (works for now)

	// First things first:
	// extract the command line flags that contain relevant matrix options.
	if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
		return usage(argv[0]);
	}

	// Initialize RGBMatrix
	RGBMatrix* matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
	if (matrix == NULL) return 1;

    // TODO: Is this off_screen_canvas necessary?
	//FrameCanvas* off_screen_canvas = matrix->CreateFrameCanvas();

	// Initialize MatrixModule static variables
	MatrixModule::InitStaticMatrixVariables(matrix);

	// Initialize the MatrixModule objects
    t_module* t_weather_module = new t_module();
	MatrixModule* weatherModule = new WeatherStation::WeatherStationModule(t_weather_module, matrix);
    t_weather_module->state = INACTIVE;
    
    t_module* t_clock_module = new t_module();
	MatrixModule* clockModule = new ClockModule(t_clock_module, matrix);
    t_clock_module->state = ACTIVE;

	// Set up an interrupt handler to be able to stop animations while they go
	// on. Each demo tests for while (!interrupt_received) {},
	// so they exit as soon as they get a signal.
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	printf("Press <CTRL-C> to exit and reset LEDs\n");

    // Run the module threads
    pthread_t clockModuleThread;
    pthread_create(&clockModuleThread, NULL, MatrixModule::Run, clockModule);
    // TODO: Uncomment the below when ready to run the weather module...
    //pthread_t weatherModuleThread;
    //pthread_create(&weatherModuleThread, NULL, MatrixModule::Run, weatherModule);

    // TODO: Find a way to succesfully suspend execution of a module when it's inactive.
    //          I think a good way to do this might be to use std::conditional_value

	// ~~~ MAIN LOOP ~~~ //
	while (!interrupt_received) {
        // Update the canvas only if the module has posted an update
        if (t_clock_module->update) {
            // TODO: local off_screen_canvas necessary?
            /*off_screen_canvas = */matrix->SwapOnVSync(t_clock_module->off_screen_canvas);
            t_clock_module->update = false;
        }

        // NOTE: Without this sleep, things are rather unstable.
        usleep(SLEEP);
	}
	// ~~~ END ~~~ //

	// TODO: Make sure you're deleting everything that needs to be deleted
	// delete weatherModule;
	delete clockModule;
    delete t_clock_module;
    delete weatherModule;
    delete t_weather_module;
	delete matrix;

	printf("Received CTRL-C. Exiting.\n");
	return 0;
}
