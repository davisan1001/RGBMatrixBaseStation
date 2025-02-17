#include <signal.h>
#include <stdio.h>
#include "led-matrix.h"

#include "clock-module.hpp"
#include "weather-station-module.hpp"

#define REFRESH_RATE 90

struct timespec current_time;
struct timespec next_time;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

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
        rgb_matrix::PrintMatrixFlags(stderr);
		return 1;
	}

	// Initialize RGBMatrix
	RGBMatrix* matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
	if (matrix == NULL) return 1;

	// Initialize MatrixModule static variables
	MatrixModule::InitStaticMatrixVariables(matrix);

	// Initialize the MatrixModule objects
	MatrixModule* weatherModule = new WeatherStation::WeatherStationModule(matrix);
	MatrixModule* clockModule = new ClockModule(matrix, true);

    // Setup module tracking for display
    //  1 = Clock Module;
    //  2 = Weather Module;
    int currentActiveModule = 1;
    MatrixModule* currentModule = clockModule;

    clock_gettime(CLOCK_REALTIME, &current_time);
    next_time.tv_sec = current_time.tv_sec + 15; // Change to the next module after 15 seconds

    // Set up an interrupt handler to be able to stop animations while they go on.
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	printf("Press <CTRL-C> to exit and reset LEDs\n");

	// ~~~ MAIN LOOP ~~~ //
	while (!interrupt_received) {
        clock_gettime(CLOCK_REALTIME, &current_time);

        if (current_time.tv_sec >= next_time.tv_sec) {
            if (currentActiveModule == 1) {
                currentActiveModule = 2;
                currentModule = weatherModule;
                next_time.tv_sec = current_time.tv_sec + 45; // Change to the next module after 45 seconds
            } else {
                currentActiveModule = 1;
                currentModule = clockModule;
                next_time.tv_sec = current_time.tv_sec + 15; // Change to the next module after 15 seconds
            }
        }
        
        // Update the canvas only if the module has posted an update
        matrix->SwapOnVSync(currentModule->Update());
	}
	// ~~~ END ~~~ //

	// Delete all objects initialized with 'new'
	delete clockModule;
    delete weatherModule;
	delete matrix;

	printf("Received CTRL-C. Exiting.\n");
	return 0;
}
