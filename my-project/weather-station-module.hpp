#ifndef WEATHER_STATION_MODULE_H
#define WEATHER_STATION_MODULE_H

#include "matrix-module.hpp"

#include <string>

class WeatherStationModule : public MatrixModule {
private:
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

	// 7-8 x 13-14 font should work here. Experiment with
	// different ones and see which you like best.
	rgb_matrix::Font current_temp_font;

	struct tm* local_datetime;

    // Kentville AAFC citypage_weather XML document
    std::string weatherCanadaDatamartURL;
    std::string weatherDataOutputFile;

    // Weather Fetch Functions
    void FetchWeatherCanData();

    void ParseWeatherXMLData();

    // Draw Methods
	void DrawSeperatorLines(); // TODO: Implement this

	void DrawCurrentDateTime(); // TODO: Implement this

	void DrawCurrentDayWeatherData(); // TODO: Implement this

	void DrawPredictedDailyForecastData(); // TODO: Implement this

public:
	WeatherStationModule(rgb_matrix::RGBMatrix* m);
	rgb_matrix::FrameCanvas* UpdateCanvas(); // TODO: Implement this
};

#endif
