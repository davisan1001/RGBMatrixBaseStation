// TODO: Complete this refactor and work on this module.
// TODO: Add a pragma ifndef directive
#include "matrix-module.hpp"

class WeatherStationModule : public MatrixModule {
  int letter_spacing = 0;
  rgb_matrix::Color seperator_color;
  rgb_matrix::Color temp_high_color;
  rgb_matrix::Color temp_low_color;
  rgb_matrix::Color clock_color;
  rgb_matrix::Color date_color;
  rgb_matrix::Color current_weekday_color;
  rgb_matrix::Color future_weekday_color;
  rgb_matrix::Color temp_predicted_low_color;
  rgb_matrix::Color temp_predicted_high_color;

  rgb_matrix::Font
	  current_temp_font;  // 7-8 x 13-14 font should work here. Experiment with
						  // different ones and see which you like best.

  struct tm *local_datetime;

  void DrawSeperatorLines() {
	// TODO: Implement this
  }

  void DrawCurrentDateTime() {
	// TODO: Implement this
  }

  void DrawCurrentDayWeatherData() {
	// TODO: Implement this
  }

  void DrawPredictedDailyForecastData() {
	// TODO: Implement this
  }

 public:
  WeatherStationModule(rgb_matrix::RGBMatrix *m) : MatrixModule(m) {
	// Setup default colors
	seperator_color = rgb_matrix::Color(84, 84, 84);
	temp_high_color = rgb_matrix::Color(255, 126, 0);
	temp_low_color = rgb_matrix::Color(0, 183, 239);
	clock_color = rgb_matrix::Color(255, 255, 255);
	date_color = rgb_matrix::Color(
		120, 120, 120);  // Grey (consider changing for visibility)
	current_weekday_color = rgb_matrix::Color(111, 49, 152);
	future_weekday_color = rgb_matrix::Color(
		120, 120, 120);  // Grey (consider changing for visibility)
	temp_predicted_low_color = rgb_matrix::Color(
		161, 161, 161);  // Grey (consider changing for visibility)
	temp_predicted_high_color = rgb_matrix::Color(
		120, 120, 120);  // Grey (consider changing for visibility);

	// TODO: Setup current temp font
  }

  rgb_matrix::FrameCanvas *UpdateCanvas() {
	// TODO: Do stuff
  }
};