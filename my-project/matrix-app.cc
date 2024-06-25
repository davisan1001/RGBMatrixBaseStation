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
  // TODO: Add all variables that need to be used by all modules here.
  // TODO: I think the font should be a private static member of this class
  //       so that all child classes have access to it.
};

// Simple generator that pulses through RGB and White.
class WeatherStationModule : public MatrixModule {
 public:
  WeatherStationModule() : MatrixModule() {
    // Do nothing for now
  }

 private:
};

// Simple analog/digital clock
class ClockModule : public MatrixModule {
 public:
  ClockModule() : MatrixModule() {
    // Do nothing for now
  }

 private:
};

// Text displaying module for testing purposes
class TextTestModule : public MatrixModule {
 public:
  TextTestModule(RGBMatrix *m) : MatrixModule() {
    off_screen_canvas = m->CreateFrameCanvas();
    // Do nothing for now
  }

  FrameCanvas UpdateCanvas() {}

 private:
  FrameCanvas *off_screen_canvas;
};

void updateFrame() {
  // Nothing for now...
}

// TODO:  Maybe I should have this initialize a base matrix-app class (or maybe
//        the MatrixModule class) that runs the matrix instead of a loop in this
//        main function. Maybe the loop should exist in the Run function of the
//        class?? Nevermind, maybe the loop should just be here in the main
//        function loop.
int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;

  // These are the defaults when no command-line flags are given.
  matrix_options.rows = 64;
  matrix_options.cols = 64;
  matrix_options.pixel_mapper_config = "rotate:90";
  matrix_options.chain_length = 1;
  matrix_options.parallel = 1;

  // First things first:
  // extract the command line flags that contain relevant matrix options.
  if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  // Setup font // TODO: Change this file to tom-thumb_fixed_4x6.bdf ??
  const char *bdf_font_file = "../fonts/tom-thumb.bdf";

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

  // The MatrixModule objects are filling the matrix continuously.
  // MatrixModule *weatherModule = new WeatherStationModule();
  MatrixModule *TextTestModule =
      new TextTestModule(off_screen_canvas);  // TODO: Why is this an error?

  // Set up an interrupt handler to be able to stop animations while they go
  // on. Each demo tests for while (!interrupt_received) {},
  // so they exit as soon as they get a signal.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("Press <CTRL-C> to exit and reset LEDs\n");

  // TODO: Move this into the class
  // Draw text on the canvas (Temporary testing)
  int text_start_x = 0;
  int text_start_y = 0;
  int letter_spacing = 0;
  Color text_color(255, 255, 255);
  char line1[1024] = "Hello World!";
  char line2[1024] = "Time: 12:45";

  DrawText(off_screen_canvas, font, text_start_x,
           text_start_y + font.baseline(), text_color, NULL, line1,
           letter_spacing);

  text_start_y += font.height();

  DrawText(off_screen_canvas, font, text_start_x,
           text_start_y + font.baseline(), text_color, NULL, line2,
           letter_spacing);

  // ~~~ MAIN LOOP ~~~
  while (!interrupt_received) {
    usleep(5 * 1000);

    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
  }
  // ~~~ END ~~~

  // TODO: Make sure you're deleting everything that needs to be deleted
  // delete weatherModule;
  delete TextTestModule;
  delete matrix;
  // delete canvas;

  printf("Received CTRL-C. Exiting.\n");
  return 0;
}