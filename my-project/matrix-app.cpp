// TODO:  I'm not sure how my program will support animations like
//        sliding where potentially 2 MatrixModules will be displayed
//        simultaneously for a few seconds as one slides to replace the first...
//        Doing this may require a redesign...
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>  // Need (Used for debugging)
// #include <chrono>     // Need [for clock]
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
	matrix_options.limit_refresh_rate_hz = 90;
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

	FrameCanvas* off_screen_canvas = matrix->CreateFrameCanvas();

	// Initialize MatrixModule static variables
	MatrixModule::InitStaticMatrixVariables(matrix);

	// Initialize the MatrixModule objects
	//MatrixModule* weatherModule = new WeatherStation::WeatherStationModule(matrix);
	MatrixModule* clockModule = new ClockModule(matrix);

	// Set up an interrupt handler to be able to stop animations while they go
	// on. Each demo tests for while (!interrupt_received) {},
	// so they exit as soon as they get a signal.
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	printf("Press <CTRL-C> to exit and reset LEDs\n");

	// ~~~ MAIN LOOP ~~~ //
	while (!interrupt_received) {
		off_screen_canvas = clockModule->UpdateCanvas();
        //off_screen_canvas = weatherModule->UpdateCanvas();

        //  TODO: Depending on implementation, check if the off_screen_canvas has been set to null...
        //      This way we know that the module had nothing to update and we can skip a call to SwapOnVSync()
		off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
	}
	// ~~~ END ~~~ //

	// TODO: Make sure you're deleting everything that needs to be deleted
	// delete weatherModule;
	delete clockModule;
	delete matrix;

	printf("Received CTRL-C. Exiting.\n");
	return 0;
}
