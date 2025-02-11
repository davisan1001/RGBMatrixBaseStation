#ifndef CLOCK_MODULE_H // Include guard
#define CLOCK_MODULE_H

#include "matrix-module.hpp"
#include "clock-module-images.hpp"

using namespace Matrix;

// Simple analog/digital clock
class ClockModule : public MatrixModule {
private:
    int letter_spacing = 0;
    rgb_matrix::Color text_color;
    rgb_matrix::Color clock_color;

    // Time Variables
    struct timespec next_time;
    struct tm local_time;

    bool flag_include_digital_clock;

    int clock_text_canvas_offset_x = 22;
    int clock_text_canvas_offset_y = 28;
    // Warning: grabbing matrix->width() and height() might result in tearing

    int hour_hand_circle_radius = 18;
    int minute_hand_circle_radius = 22;
    int second_hand_circle_radius = 20;

    int circle_center_x = (matrix_width - 1) / 2;
    int circle_center_y = (matrix_height - 1) / 2;

    // TODO:    The current implementation will probably cause
    //          A problem when I stop updating this module...
    //          The next_time.tv_sec will stop counting up and retain it's old value.
    //          A solution to this would be to create a function that automatically
    //          calls after the module status has been idle for a while.
    //          In this functtion we would reset the next_time.tv_sec to time(null), just
    //          like in the constructor.
    void SetCurrentNetworkTime();

    void DrawClockHourHand(double hour_fraction);
    void DrawClockMinHand(double minute_fraction);
    void DrawClockSecHand(double second_fraction);
    void DrawClock();

    void DrawDigitalClock();

    void *Main();

public:
    ClockModule(t_module* t_modArg, rgb_matrix::RGBMatrix* m);
};

#endif
