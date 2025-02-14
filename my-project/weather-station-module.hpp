#ifndef WEATHER_STATION_MODULE_H
#define WEATHER_STATION_MODULE_H

#include "matrix-module.hpp"

#include <string>

using namespace Matrix;

namespace WeatherStation {

    typedef enum {
        SUN,            // Icon Exists (both)
        PARTLY_CLOUDY,
        MOSTLY_CLOUDY,  // Icon Exists (both)
        LIGHT_FLURRIES,
        SNOW,
        CLOUD,          // Icon Exists (small only)
        LIGHT_RAIN,
        RAIN,           // Icon Exists (both)
        FREEZING_RAIN,
        RAIN_SNOW,
        THUNDERSHOWERS, // Icon Exists (both)
        UNKNOWN
    } WeatherType;

    extern std::string weatherTypeString[12];

    typedef struct {
        std::string day;

        bool isNight;

        double tempCur; // Only available for current 
        double windChill; // Only available for current conditions

        double tempHigh;

        // Abbreviated
        WeatherType type = UNKNOWN; // Default to UNKNOWN
        std::string textSummary;
        int pop; // Percentage of Precipitation. May be empty, so set to -1.
    } WeatherDay;

    typedef struct {
        // Date and Time of the Weather data
        std::string year;
        std::string month;
        std::string day;
        std::string hour;
        std::string minute;

        std::string sunrise;
        std::string sunset;

        WeatherDay currentConditions;

        WeatherDay forecast[4];
    } Weather;

    class WeatherStationModule : public MatrixModule {
    private:
        int letter_spacing = 0;
        rgb_matrix::Color seperator_color;
        rgb_matrix::Color temp_cur_color;
        rgb_matrix::Color temp_high_color;
        rgb_matrix::Color windchill_color;
        rgb_matrix::Color clock_color;
        rgb_matrix::Color date_color;
        rgb_matrix::Color current_weekday_color;
        rgb_matrix::Color future_weekday_color;
        rgb_matrix::Color temp_predicted_low_color;
        rgb_matrix::Color temp_predicted_high_color;

        // 7-8 x 13-14 font should work here. Experiment with
        // different ones and see which you like best.
        rgb_matrix::Font current_temp_font;
        // Everything else can use the default font

        // Time Variables
        struct timespec next_time;
        struct tm local_time;

        // TODO: There is another server that serves the same information. If this one returns an error, try again with the other one.
        // TODO: Need to handle errors... I think the weatherCAN site goes down often... It's certainly not reliable.
        std::string weatherCanadaStationCode = "s0000439";
        std::string weatherCanadaDatamartURL = "https://dd.weather.gc.ca/citypage_weather/xml/NS/" + weatherCanadaStationCode + "_e.xml";
        std::string weatherDataFile = "weatherData.xml";
        std::string weatherArchivedDataFile = "weatherDataArchive.xml";

        // Hold the weather data
        Weather weather;

        // Weather Fetch Functions
        static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
        std::string FetchData(std::string& url);
        WeatherDay FetchArchivedForecast(); // TODO: Change the working of this...

        // Weather Functions
        WeatherType extractWeatherType(int iconCode, std::string textSummary);
        void ParseWeatherCanXMLData();

        // Time Functions
        void SetCurrentNetworkTime();

        // Draw Methods
        const uint8_t* GetLargeImageByType(WeatherType type);
        const uint8_t* GetSmallImageByType(WeatherType type);

        void DrawSeperatorLines();

        void DrawCurrentDateTime();

        void DrawCurrentDayWeatherData(); // TODO: Implement this

        void DrawPredictedDailyForecastData(); // TODO: Implement this

        void DrawWeatherStationCanvas(bool dateTimeOnly); // Main draw function

        // Main Method
        void* Main(); // TODO: Implement this

    public:
        WeatherStationModule(t_module* t_modArg, rgb_matrix::RGBMatrix* m);
    };

} // namespace WeatherStation 

#endif
