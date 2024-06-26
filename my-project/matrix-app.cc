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

#include "graphics.h"
#include "led-matrix.h"
#include "matrix-images.hpp"  // Need [contains byte array images]
#include "pixel-mapper.h"

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

// ~~~ BEGIN CLASS DEFINITIONS ~~~ //
class MatrixModule {
 protected:
  rgb_matrix::RGBMatrix *matrix;
  rgb_matrix::FrameCanvas *off_screen_canvas;

  rgb_matrix::Font font;

  MatrixModule() {}
  MatrixModule(rgb_matrix::RGBMatrix *m) {
    // Setup font
    const char *bdf_font_file = "../fonts/tom-thumb_fixed_4x6.bdf";
    // const char *bdf_font_file = "../fonts/tom-thumb.bdf";

    if (bdf_font_file == NULL) {
      std::string errMsg = std::string("Unrecognized font file\n");
      std::cerr << errMsg.c_str();
      throw std::invalid_argument(errMsg);
    }

    // Load font. This needs to be a filename with a bdf bitmap font.
    if (!font.LoadFont(bdf_font_file)) {
      std::string errMsg =
          std::string("Couldn't load font \'") + bdf_font_file + "\'\n";
      std::cerr << errMsg.c_str();
      throw std::invalid_argument(errMsg);
    }

    matrix = m;

    off_screen_canvas = m->CreateFrameCanvas();
  }

 public:
  virtual ~MatrixModule() {}
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
  int letter_spacing = 0;
  rgb_matrix::Color text_color;
  rgb_matrix::Color clock_color;

  int clock_text_canvas_offset_x = 22;  // TODO: Consider moving this over by -1
  int clock_text_canvas_offset_y = 28;
  int image_width = MatrixModule::matrix->width();
  int image_height = MatrixModule::matrix->height();
  // Warning: grabbing matrix->width() and height() might result in tearing

  int hour_hand_circle_radius = 24;
  int minute_hand_circle_radius = 18;
  int second_hand_circle_radius = 16;
  int circle_center_x = (image_width - 1) / 2;
  int circle_center_y = (image_height - 1) / 2;

  void DrawClock(struct tm *time) {
    /* ~~ Draw ticks around the perimeter of the screen ~~ */
    rgb_matrix::SetImage(off_screen_canvas, 0, 0,
                         matrix_images::analog_clock_base,
                         matrix_images::analog_clock_base_size,
                         matrix_images::analog_clock_base_width,
                         matrix_images::analog_clock_base_height, false);

    // Get fraction of hour
    double hour_fraction = (double)(time->tm_hour % 12) / 12;
    hour_fraction = 0 - hour_fraction;

    // Get fraction of minute
    double minute_fraction = (double)time->tm_min / 60;
    minute_fraction = 0 - minute_fraction;

    // Get fraction of second
    double second_fraction = (double)time->tm_sec / 60;
    second_fraction = 0 - second_fraction;

    // ~~ Draw hour line ~~ //
    // Calculate point on circle circumference
    int hour_end_x =
        circle_center_x +
        (hour_hand_circle_radius * cos(hour_fraction * (2 * std::numbers::pi) +
                                       (0.5 * std::numbers::pi)));
    int hour_end_y =
        circle_center_y +
        (hour_hand_circle_radius * sin(hour_fraction * (2 * std::numbers::pi) -
                                       (0.5 * std::numbers::pi)));

    // Need to draw 4 lines because the middle is represented as a 2 by 2 pixel
    // block
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        rgb_matrix::DrawLine(off_screen_canvas, circle_center_x + i,
                             circle_center_y + j, hour_end_x + i,
                             hour_end_y + j, clock_color);
      }
    }

    // ~~ Draw minute line ~~ //
    // Calculate point on circle circumference
    int minute_end_x =
        circle_center_x + (minute_hand_circle_radius *
                           cos(minute_fraction * (2 * std::numbers::pi) +
                               (0.5 * std::numbers::pi)));
    int minute_end_y =
        circle_center_y + (minute_hand_circle_radius *
                           sin(minute_fraction * (2 * std::numbers::pi) -
                               (0.5 * std::numbers::pi)));

    // Need to draw 4 lines because the middle is represented as a 2 by 2 pixel
    // block
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        rgb_matrix::DrawLine(off_screen_canvas, circle_center_x + i,
                             circle_center_y + j, minute_end_x + i,
                             minute_end_y + j, clock_color);
      }
    }

    // ~~ Draw second line ~~ //
    // Calculate point on circle circumference
    int second_end_x =
        circle_center_x + (second_hand_circle_radius *
                           cos(second_fraction * (2 * std::numbers::pi) +
                               (0.5 * std::numbers::pi)));
    int second_end_y =
        circle_center_y + (second_hand_circle_radius *
                           sin(second_fraction * (2 * std::numbers::pi) -
                               (0.5 * std::numbers::pi)));

    // Need only draw one line because this is the second hand.
    rgb_matrix::DrawLine(off_screen_canvas, circle_center_x, circle_center_y,
                         second_end_x, second_end_y, clock_color);

    // ~~ Erase digital clock bounding box ~~ //
    // (set all pixel values to black) in the square where the time will go
    rgb_matrix::SetImage(off_screen_canvas, clock_text_canvas_offset_x,
                         clock_text_canvas_offset_y,
                         matrix_images::digital_clock_bbox_erase,
                         matrix_images::digital_clock_bbox_erase_size,
                         matrix_images::digital_clock_bbox_erase_width,
                         matrix_images::digital_clock_bbox_erase_height, false);

    // ~~ Draw text in the center of the screen //
    std::string hours = std::to_string(time->tm_hour);
    std::string minutes = std::to_string(time->tm_min);
    if (time->tm_hour < 10) {
      hours = "0" + std::to_string(time->tm_hour);
    }
    if (time->tm_min < 10) {
      minutes = "0" + std::to_string(time->tm_min);
    }

    std::string time_str = hours + ":" + minutes;
    rgb_matrix::DrawText(off_screen_canvas, font,
                         clock_text_canvas_offset_x + 1,
                         clock_text_canvas_offset_y + 1 + font.baseline(),
                         text_color, NULL, time_str.c_str(), letter_spacing);
  }

 public:
  ClockModule(rgb_matrix::RGBMatrix *m) : MatrixModule(m) {
    // Set text color default to white
    text_color = rgb_matrix::Color(255, 255, 255);

    // Set clock color default to white
    clock_color = rgb_matrix::Color(255, 255, 255);
  }

  rgb_matrix::FrameCanvas *UpdateCanvas() {
    // current date and time on the current system
    time_t now = time(0);

    // convert now to local time
    struct tm *local_time = localtime(&now);

    // TODO: draw the analog clock with the digital clock in the center.
    DrawClock(local_time);

    // FOR DEBUGGING PURPOSES
    // convert local_time to string form
    char *date_time = asctime(local_time);
    std::cout << "The current date and time is: " << date_time << std::endl;

    return off_screen_canvas;
  }
};

// Text displaying module for testing purposes
class TextTestModule : public MatrixModule {
  int text_start_x = 0;
  int text_start_y = 0;
  int letter_spacing = 0;
  rgb_matrix::Color text_color;
  std::string displayText;

  void ResetTextStartValues() {
    text_start_x = 0;
    text_start_y = 0;
  }

 public:
  TextTestModule(rgb_matrix::RGBMatrix *m) : MatrixModule(m) {
    // Set text color default to white
    text_color = rgb_matrix::Color(255, 255, 255);

    // Set default text
    displayText = "Hello World!\nTime: 12:45";
  }

  void setText(std::string text) { displayText = text; }

  void setColor(uint8_t rr, uint8_t gg, uint8_t bb) {
    text_color = rgb_matrix::Color(rr, gg, bb);
  }

  rgb_matrix::FrameCanvas *UpdateCanvas() {
    std::stringstream streamText(displayText);
    std::string line;
    while (std::getline(streamText, line)) {
      rgb_matrix::DrawText(off_screen_canvas, font, text_start_x,
                           text_start_y + font.baseline(), text_color, NULL,
                           line.c_str(), letter_spacing);

      text_start_y += font.height();
    }

    ResetTextStartValues();

    return off_screen_canvas;
  }
};

// ~~~ BEGIN FUNCTION DEFINITIONS ~~~ //
static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options>\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t<No Options Set Yet>              : N/A\n");

  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

int main(int argc, char *argv[]) {
  using namespace rgb_matrix;

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

  // Initialize RGBMatrix
  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) return 1;

  FrameCanvas *off_screen_canvas = matrix->CreateFrameCanvas();

  // Initialize the MatrixModule objects
  // WeatherModule *weatherModule = new WeatherStationModule();
  TextTestModule *textTestModule = new TextTestModule(matrix);
  ClockModule *clockModule = new ClockModule(matrix);

  // Set up an interrupt handler to be able to stop animations while they go
  // on. Each demo tests for while (!interrupt_received) {},
  // so they exit as soon as they get a signal.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("Press <CTRL-C> to exit and reset LEDs\n");

  // ~~~ MAIN LOOP ~~~ //
  while (!interrupt_received) {
    usleep(1 * 1000000);  // Sleep for 1 second

    // TODO: Weird flickering occurs on update. Not sure why...

    // off_screen_canvas = textTestModule->UpdateCanvas();
    off_screen_canvas = clockModule->UpdateCanvas();

    off_screen_canvas = matrix->SwapOnVSync(off_screen_canvas);
  }
  // ~~~ END ~~~ //

  // TODO: Make sure you're deleting everything that needs to be deleted
  // delete weatherModule;
  delete textTestModule;
  delete clockModule;
  delete matrix;

  printf("Received CTRL-C. Exiting.\n");
  return 0;
}