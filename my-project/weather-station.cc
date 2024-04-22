#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

#include "graphics.h"
#include "led-matrix.h"
#include "pixel-mapper.h"

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options>\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t<No Options Set Yet>              : N/A\n");

  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

class MatrixModule {
 protected:
  MatrixModule() {}

 public:
  virtual ~MatrixModule() {}

 private:
  // TODO: No private members yet
};

// Simple generator that pulses through RGB and White.
class WeatherStationModule : public MatrixModule {
 public:
  WeatherStationModule() : MatrixModule() {
    // Do nothing for now
  }

 private:
};

void updateFrame() {
  // Nothing for now...
}

int main(int argc, char *argv[]) {
  const char *demo_parameter = NULL;
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;

  // These are the defaults when no command-line flags are given.
  matrix_options.rows = 64;
  matrix_options.cols = 64;
  matrix_options.chain_length = 1;
  matrix_options.parallel = 1;

  // First things first: extract the command line flags that contain
  // relevant matrix options.
  if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  // Setup font
  const char *bdf_font_file = "../fonts/tom-thumb-fixed_4x6.bdf";
  int x_orig = 0;  // TODO: No need?
  int y_orig = 0;  // TODO: No need?
  int letter_spacing = 0;

  if (bdf_font_file == NULL) {
    fprintf(stderr, "Unrecognized font file\n");
    return 1;
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  // Initialize RGBMatrix
  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) return 1;

  FrameCanvas *off_screen_canvas = matrix->CreateFrameCanvas();

  // The MatrixModule objects are filling
  // the matrix continuously.
  MatrixModule *weatherModule = new WeatherStationModule();

  // Set up an interrupt handler to be able to stop animations while they go
  // on. Each demo tests for while (!interrupt_received) {},
  // so they exit as soon as they get a signal.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("Press <CTRL-C> to exit and reset LEDs\n");

  int text_start_x = 0;
  int text_start_y = 0;
  Color text_color(255, 255, 0);
  char line[1024] = "Hello World!\n Time: 12:45";

  DrawText(off_screen_canvas, font, text_start_x,
           text_start_y + font.baseline(), text_color, NULL, line,
           letter_spacing);

  // off_screen_canvas->
  off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);

  // ~~~ MAIN LOOP ~~~
  while (!interrupt_received) {
    usleep(5 * 1000);
  }
  // ~~~ END ~~~

  delete weatherModule;
  // delete canvas;

  printf("Received CTRL-C. Exiting.\n");
  return 0;
}