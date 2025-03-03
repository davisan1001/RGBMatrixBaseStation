#include "clock-module.hpp"

#include <math.h>
#include <numbers>

// Simple analog/digital clock

ClockModule::ClockModule(rgb_matrix::RGBMatrix* m, bool includeDigitalClock) : MatrixModule(m) {
	// Set text color default to white
	text_color = rgb_matrix::Color(255, 255, 255);

	// Set clock color default to white
	clock_color = rgb_matrix::Color(255, 255, 255);

	// Set default flag_include_digital_clock
	flag_include_digital_clock = includeDigitalClock;
}

void ClockModule::SetCurrentNetworkTime() {
	// current date and time on the current system
	next_time.tv_sec = time(NULL);
	next_time.tv_nsec = 0;
}

void ClockModule::DrawClockHourHand(double hour_fraction) {
	// Calculate point on circle circumference
	int hour_end_x = round(
		circle_center_x + (hour_hand_circle_radius *
			cos(hour_fraction * (2.0 * std::numbers::pi) + (0.5 * std::numbers::pi))));
	int hour_end_y = round(
		circle_center_y + (hour_hand_circle_radius *
			sin(hour_fraction * (2.0 * std::numbers::pi) - (0.5 * std::numbers::pi))));

	// Need to draw 4 lines because the middle is represented as a 2 by 2 pixel block
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			rgb_matrix::DrawLine(off_screen_canvas, circle_center_x + i,
				circle_center_y + j, hour_end_x + i,
				hour_end_y + j, clock_color);
		}
	}
}

void ClockModule::DrawClockMinHand(double minute_fraction) {
	// Calculate point on circle circumference
	int minute_end_x = round(
		circle_center_x + (minute_hand_circle_radius *
			cos(minute_fraction * (2 * std::numbers::pi) + (0.5 * std::numbers::pi))));
	int minute_end_y = round(
		circle_center_y + (minute_hand_circle_radius *
			sin(minute_fraction * (2 * std::numbers::pi) - (0.5 * std::numbers::pi))));

	// Need to draw 4 lines because the middle is represented as a 2 by 2 pixel
	// block
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			rgb_matrix::DrawLine(off_screen_canvas, circle_center_x + i,
				circle_center_y + j, minute_end_x + i,
				minute_end_y + j, clock_color);
		}
	}
}

void ClockModule::DrawClockSecHand(double second_fraction) {
	// Calculate point on circle circumference
	int second_end_x = round(
		circle_center_x + (second_hand_circle_radius *
			cos(second_fraction * (2.0 * std::numbers::pi) + (0.5 * std::numbers::pi))));
	int second_end_y = round(
		circle_center_y + (second_hand_circle_radius *
			sin(second_fraction * (2.0 * std::numbers::pi) - (0.5 * std::numbers::pi))));

	// Need only draw one line because this is the second hand.
	rgb_matrix::DrawLine(off_screen_canvas, circle_center_x, circle_center_y,
		second_end_x, second_end_y, clock_color);
}

void ClockModule::DrawClock() {
    // Fill canvas blank
    off_screen_canvas->Fill(0, 0, 0);

	// Draw ticks around the perimeter of the screen
	rgb_matrix::SetImage(off_screen_canvas, 0, 0,
		matrix_images::analog_clock_base,
		matrix_images::analog_clock_base_size,
		matrix_images::analog_clock_base_width,
		matrix_images::analog_clock_base_height, false);

	// Get fraction of hour
	double hour_fraction = (double)((local_time.tm_hour % 12) + (double)(local_time.tm_min) / 60) / 12;
	hour_fraction = 0 - hour_fraction;

	// Get fraction of minute
	double minute_fraction = (double)local_time.tm_min / 60;
	minute_fraction = 0 - minute_fraction;

	// Get fraction of second
	double second_fraction = (double)local_time.tm_sec / 60;
	second_fraction = 0 - second_fraction;

	// ~~ Draw hour line ~~ //
	DrawClockHourHand(hour_fraction);

	// ~~ Draw minute line ~~ //
	DrawClockMinHand(minute_fraction);

	// ~~ Draw second line ~~ //
	DrawClockSecHand(second_fraction);

	if (flag_include_digital_clock) {
		DrawDigitalClock();
	}
}

void ClockModule::DrawDigitalClock() {
	// ~~ Erase digital clock bounding box ~~ //
	// (set all pixel values to black) in the square where the local_time will
	// go
	rgb_matrix::SetImage(off_screen_canvas, clock_text_canvas_offset_x,
		clock_text_canvas_offset_y,
		matrix_images::digital_clock_bbox_erase,
		matrix_images::digital_clock_bbox_erase_size,
		matrix_images::digital_clock_bbox_erase_width,
		matrix_images::digital_clock_bbox_erase_height, false);

	// ~~ Draw text in the center of the screen //
    int local_hour = (local_time.tm_hour % 12) == 0 ? 12 : (local_time.tm_hour % 12); // Convert 24 hour time to 12 hour time
	int local_minute = local_time.tm_min;
	std::string hours = std::to_string(local_hour);
	std::string minutes = std::to_string(local_minute);
	if (local_hour < 10) {
		hours = "0" + std::to_string(local_hour);
	}
	if (local_minute < 10) {
		minutes = "0" + std::to_string(local_minute);
	}

	std::string local_time_str = hours + ":" + minutes;
	rgb_matrix::DrawText(
		off_screen_canvas, font, clock_text_canvas_offset_x + 1,
		clock_text_canvas_offset_y + 1 + font.baseline(), text_color, NULL,
		local_time_str.c_str(), letter_spacing);
}

rgb_matrix::FrameCanvas* ClockModule::Update() {
    // Update the time seconds
    SetCurrentNetworkTime();
    next_time.tv_sec += 1;

    // Set readable local_time from next_time.tv_sec
    localtime_r(&next_time.tv_sec, &local_time);

    // Draw the clock (using the local_time set from the next time).
    DrawClock();

    // Wait to update time.
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, NULL);
    
    // Set update ready to true
    return off_screen_canvas;
}
