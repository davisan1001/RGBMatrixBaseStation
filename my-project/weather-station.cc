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

  int opt;
  while ((opt = getopt(argc, argv, "dD:r:P:c:p:b:m:LR:")) != -1) {
    switch (opt) {
      case 'D':
        demo = atoi(optarg);
        break;

      case 'm':
        scroll_ms = atoi(optarg);
        break;

      default: /* '?' */
        return usage(argv[0]);
    }
  }

  if (optind < argc) {
    demo_parameter = argv[optind];
  }

  if (demo < 0) {
    fprintf(stderr, TERM_ERR "Expected required option -D <demo>\n" TERM_NORM);
    return usage(argv[0]);
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) return 1;

  printf("Size: %dx%d. Hardware gpio mapping: %s\n", matrix->width(),
         matrix->height(), matrix_options.hardware_mapping);

  Canvas *canvas = matrix;

  // The DemoRunner objects are filling
  // the matrix continuously.
  DemoRunner *demo_runner = NULL;
  switch (demo) {
    case 0:
      demo_runner = new RotatingBlockGenerator(canvas);
      break;

    case 1:
    case 2:
      if (demo_parameter) {
        ImageScroller *scroller =
            new ImageScroller(matrix, demo == 1 ? 1 : -1, scroll_ms);
        if (!scroller->LoadPPM(demo_parameter)) return 1;
        demo_runner = scroller;
      } else {
        fprintf(stderr, "Demo %d Requires PPM image as parameter\n", demo);
        return 1;
      }
      break;

    case 3:
      demo_runner = new SimpleSquare(canvas);
      break;

    case 4:
      demo_runner = new ColorPulseGenerator(matrix);
      break;

    case 5:
      demo_runner = new GrayScaleBlock(canvas);
      break;

    case 6:
      demo_runner = new Sandpile(canvas, scroll_ms);
      break;

    case 7:
      demo_runner = new GameLife(canvas, scroll_ms);
      break;

    case 8:
      demo_runner = new Ant(canvas, scroll_ms);
      break;

    case 9:
      demo_runner = new VolumeBars(canvas, scroll_ms, canvas->width() / 2);
      break;

    case 10:
      demo_runner = new GeneticColors(canvas, scroll_ms);
      break;

    case 11:
      demo_runner = new BrightnessPulseGenerator(matrix);
      break;
  }

  if (demo_runner == NULL) return usage(argv[0]);

  // Set up an interrupt handler to be able to stop animations while they go
  // on. Each demo tests for while (!interrupt_received) {},
  // so they exit as soon as they get a signal.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("Press <CTRL-C> to exit and reset LEDs\n");

  // Now, run our particular demo; it will exit when it sees interrupt_received.
  demo_runner->Run();

  delete demo_runner;
  delete canvas;

  printf("Received CTRL-C. Exiting.\n");
  return 0;
}