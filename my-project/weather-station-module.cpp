#include "weather-station-module.hpp"

#include "weather-module-images.hpp"

WeatherStationModule::WeatherStationModule(rgb_matrix::RGBMatrix *m) : MatrixModule(m) {
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