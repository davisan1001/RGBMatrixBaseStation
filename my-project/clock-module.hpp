#ifndef CLOCK_MODULE_H // Include guard
#define CLOCK_MODULE_H

#include "matrix-module.hpp"
#include "clock-module-images.hpp"

// Simple analog/digital clock
class ClockModule : public MatrixModule {
private:
    int letter_spacing = 0;
    rgb_matrix::Color text_color;
    rgb_matrix::Color clock_color;

    // Time Variables
    struct timespec next_time;
    struct tm* local_time;

    bool flag_include_digital_clock;

    int clock_text_canvas_offset_x = 22;
    int clock_text_canvas_offset_y = 28;
    // Warning: grabbing matrix->width() and height() might result in tearing

    int hour_hand_circle_radius = 18;
    int minute_hand_circle_radius = 22;
    int second_hand_circle_radius = 20;

    int circle_center_x = matrix_width / 2;
    int circle_center_y = matrix_height / 2;

    void SetCurrentNetworkTime();
    void UpdateTime(); // TODO: Doesn't do anything.

    void DrawClockHourHand(double hour_fraction);
    void DrawClockMinHand(double minute_fraction);
    void DrawClockSecHand(double second_fraction);
    void DrawClock();

    void DrawDigitalClock();

public:
    ClockModule(rgb_matrix::RGBMatrix* m);

    rgb_matrix::FrameCanvas* UpdateCanvas();
};

#endif
