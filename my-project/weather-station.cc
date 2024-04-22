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

class DemoRunner {
 protected:
  DemoRunner(Canvas *canvas) : canvas_(canvas) {}
  inline Canvas *canvas() { return canvas_; }

 public:
  virtual ~DemoRunner() {}
  virtual void Run() = 0;

 private:
  Canvas *const canvas_;
};

// Simple generator that pulses through RGB and White.
class ColorPulseGenerator : public DemoRunner {
 public:
  ColorPulseGenerator(RGBMatrix *m) : DemoRunner(m), matrix_(m) {
    off_screen_canvas_ = m->CreateFrameCanvas();
  }
  void Run() override {
    uint32_t continuum = 0;
    while (!interrupt_received) {
      usleep(5 * 1000);
      continuum += 1;
      continuum %= 3 * 255;
      int r = 0, g = 0, b = 0;
      if (continuum <= 255) {
        int c = continuum;
        b = 255 - c;
        r = c;
      } else if (continuum > 255 && continuum <= 511) {
        int c = continuum - 256;
        r = 255 - c;
        g = c;
      } else {
        int c = continuum - 512;
        g = 255 - c;
        b = c;
      }
      off_screen_canvas_->Fill(r, g, b);
      off_screen_canvas_ = matrix_->SwapOnVSync(off_screen_canvas_);
    }
  }

 private:
  RGBMatrix *const matrix_;
  FrameCanvas *off_screen_canvas_;
};

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

  // Now extract any command line flags that are relevant to this program.
  // TODO: No command line flags are relevant to this program yet...
  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
      default: /* '?' */
        // return usage(argv[0]);
        break;
    }
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) return 1;

  // TODO: Purpose?... Debugging?
  printf("Size: %dx%d. Hardware gpio mapping: %s\n", matrix->width(),
         matrix->height(), matrix_options.hardware_mapping);

  Canvas *canvas = matrix;

  // The DemoRunner objects are filling
  // the matrix continuously.
  DemoRunner *demo_runner = new ColorPulseGenerator(matrix);

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