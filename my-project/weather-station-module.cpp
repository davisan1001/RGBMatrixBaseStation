#include "weather-station-module.hpp"

#include "weather-module-images.hpp"
#include <stdlib.h>
#include "pugixml.cpp"

using namespace std;

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

    weatherCanadaDatamartURL = "https://dd.weather.gc.ca/citypage_weather/xml/NS/s0000439_e.xml";
    weatherDataOutputFile = "weatherData.xml";
}

// Weather Fetch Functions
// Fetch the weather XML file using CURL from the bash command line.
void WeatherStationModule::FetchWeatherCanData() {
    string curlCommand = "curl " + weatherCanadaDatamartURL + " > " + weatherDataOutputFile;

    int status = system(curlCommand.c_str()); // This is a blocking call (which may be okay)

    if (status != 0) {
        // TODO: There was an error, so handle it.
        return;
    }

    return;
}

void WeatherStationModule::ParseWeatherXMLData() {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("tree.xml");

    std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;
    return;
}

// Draw Methods
void WeatherStationModule::DrawSeperatorLines() {
    return;
}

void WeatherStationModule::DrawCurrentDateTime() {
    return;
}

void WeatherStationModule::DrawCurrentDayWeatherData() {
    return;
}

void WeatherStationModule::DrawPredictedDailyForecastData() {
    return;
}