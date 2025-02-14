#include "weather-station-module.hpp"

#include "weather-module-images.hpp"
#include <curl/curl.h>
#include <stdexcept>
#include <stdlib.h>
#include <regex>
#include "pugixml.hpp"

using namespace std;
using namespace Matrix;
using namespace WeatherStation;

string weatherTypeString[12] = { "SUN", "PARTLY_CLOUDY", "MOSTLY_CLOUDY", "LIGHT_FLURRIES", "SNOW", "CLOUD", "LIGHT_RAIN", "RAIN", "FREEZING_RAIN", "RAIN_SNOW", "THUNDERSHOWERS", "UNKNOWN" };

WeatherStationModule::WeatherStationModule(t_module* t_modArg, rgb_matrix::RGBMatrix *m) : MatrixModule(t_modArg, m) {
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

	// Setup current temp font
	const char* bdf_font_file = "../fonts/8x13B.bdf"; // TODO: This should be setable for each matrix module.

	if (bdf_font_file == NULL) {
		std::string errMsg = std::string("Unrecognized font file\n");
		std::cerr << errMsg.c_str();
		throw std::invalid_argument(errMsg);
	}

	// Load font. This needs to be a filename with a bdf bitmap font.
	if (!current_temp_font.LoadFont(bdf_font_file)) {
		std::string errMsg =
			std::string("Couldn't load font \'") + bdf_font_file + "\'\n";
		std::cerr << errMsg.c_str();
		throw std::invalid_argument(errMsg);
	}

    // Setup the Weather data storage struct
    weather = Weather();
    // TODO: Set all values to defaults
}

// Weather Functions
WeatherType WeatherStationModule::extractWeatherType(int iconCode, std::string textSummary) {
    WeatherType eval = UNKNOWN;

    if (iconCode > -1) {
        switch (iconCode) {
        case 0:
        case 1:
            eval = SUN;
            break;
        case 2:
            eval = PARTLY_CLOUDY;
            break;
        case 3:
            eval = MOSTLY_CLOUDY;
            break;
        case 10:
            eval = CLOUD;
            break;
        case 6:
        case 11:
        case 12:
            eval = LIGHT_RAIN;
            break;
        case 13:
            eval = RAIN;
            break;
        case 14:
            eval = FREEZING_RAIN;
            break;
        case 7:
        case 15:
            eval = RAIN_SNOW;
            break;
        case 8:
            eval = LIGHT_FLURRIES;
            break;
        case 16:
        case 17:
        case 18:
            eval = SNOW;
            break;
        case 19:
        case 39:
        case 46:
        case 47:
            eval = THUNDERSHOWERS;
            break;
        default:
            eval = UNKNOWN;
            break;
        }
    }

    // TODO: If eval is still UNKNOWN then attempt to extract from textSummary.

    return eval;
}


// Weather Fetch Functions
// Callback function to write received data into a std::string
size_t WeatherStationModule::CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Fetch the weather XML using CURL.
std::string WeatherStationModule::FetchData(std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw runtime_error("Failed to initialize libcurl.");
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Disable SSL verification (not recommended for production)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw runtime_error("curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
        response = "";
    }

    curl_easy_cleanup(curl);
    return response;
}

// TODO: This is A LOT of function for just one method. refactor this by pulling out stuff and organizing into understandable sections with comments.
WeatherDay WeatherStationModule::FetchArchivedForecast() {
    // Get current time.
    // current date and time on the current system
    time_t now = time(NULL);
    // convert now to UTC
    tm *utc_time = gmtime(&now);

    string year = std::to_string(utc_time->tm_year + 1900);
    string month = std::to_string(utc_time->tm_mon + 1);
    if (utc_time->tm_mon + 1 < 10) { month = "0" + month; }
    string day = std::to_string(utc_time->tm_mday);
    if (utc_time->tm_mday < 10) { day = "0" + day; }

    string hour = std::to_string(utc_time->tm_hour);
    if (utc_time->tm_hour < 10) { hour = "0" + hour; }

    string url = std::string("https://dd.weather.gc.ca/") + year + month + day + std::string("/WXO-DD/citypage_weather/NS/");

    WeatherDay weatherDay;

    // Fetch weather archive from a couple hours ago.
    int hour_diff = 2;
    while(true) {
        // If we've gone back 8 hours and still don't have anything, then we should stop looking
        if (hour_diff > 8) {
            throw runtime_error("Weather archive pulled up to 8 hours ago, and didn't find required forecast data..."); // TODO: Create custom exceptions to properly handle these...
        }

        // Try checking 2 hours ago
        string hour = std::to_string(utc_time->tm_hour - hour_diff);
        if (utc_time->tm_hour - hour_diff < 10) { hour = "0" + hour; }
        string specific_hour_url = url + hour + std::string("/");

        // Find the weatherArchiveData File Name
        string getFilenameCmd = "curl " + specific_hour_url + " | grep -om1 '\"[^\"]*" + weatherCanadaStationCode + "_en\\.xml\"' | tr -d '\"'";

        string weatherArchiveDataFilename;
        FILE * stream;
        const int max_buffer = 256;
        char buffer[max_buffer];

        stream = popen(getFilenameCmd.c_str(), "r");
        if (stream) {
            while (!feof(stream))
                if (fgets(buffer, max_buffer, stream) != NULL) weatherArchiveDataFilename.append(buffer);
            pclose(stream);
        }

        // Remove any possible trailing newline
        weatherArchiveDataFilename = weatherArchiveDataFilename.erase(weatherArchiveDataFilename.find_last_not_of("\n") + 1);

        // Get the Weather Archive Data and Parse XML
        specific_hour_url = specific_hour_url + weatherArchiveDataFilename;
        string curlCommand = "curl " + specific_hour_url + " > " + weatherArchivedDataFile;

        int status = system(curlCommand.c_str());
        if (status != 0) {
            throw runtime_error("System Curl command returned non-zero code..."); // TODO: Create custom exceptions to properly handle these...
        }

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(weatherArchivedDataFile.c_str());

        if (!result) { // XML Parsing Error Checking
            throw runtime_error("XML parser failed..."); // TODO: Create custom exceptions to properly handle these...
        }

        // Get forecast type data
        pugi::xml_node currentForecast = doc.child("siteData").child("forecastGroup").child("forecast"); // The first forecast object should be Today or Tonight (depending on time of day).
        if (std::string(currentForecast.child("period").attribute("textForecastName").value()) == std::string("Today")) {
            // Get data and return
            weatherDay.tempHigh = currentForecast.child("temperatures").find_child_by_attribute("temperature", "class", "high").text().as_double(-100);
        
            weatherDay.pop = currentForecast.child("abbreviatedForecast").child("pop").text().as_int(-1);

            weatherDay.textSummary = currentForecast.child("abbreviatedForecast").child("textSummary").text().get();

            // Extract the Type
            int iconCode = currentForecast.child("abbreviatedForecast").child("iconCode").text().as_int(-1);
            WeatherType type = extractWeatherType(iconCode, weatherDay.textSummary);

            weatherDay.type = type;
            break;
        } else {
            // If Today's forecast doesn't exist, fetch weather archive from two hours before that. etc...
            // Increase hour_diff and try again
            hour_diff += 2;
            continue;
        }
    }

    return weatherDay;
}

void WeatherStationModule::ParseWeatherCanXMLData() {
    pugi::xml_document doc;
    pugi::xml_parse_result result;

    try {
        result = doc.load_string(FetchData(weatherCanadaDatamartURL).c_str());
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        // TODO: Handle what to do if the forecast data could not be successfully fetched.
    }

    // XML Parsing Error Checking
    if (!result) {
        throw runtime_error("Error parsing XML weather data");
    }


    // Get weather data
    pugi::xml_node siteData = doc.child("siteData"); // Get the first siteData XML node
    // If siteData node does not exist, then there was likely a WeatherCAN server error
    if (!siteData) {
        throw runtime_error("Weather XML data retrieved is incomplete... Likely due to WeatherCAN server error.");
    }

    // Get AST date info
    pugi::xml_node dateTime = siteData.find_child_by_attribute("dateTime", "zone", "AST"); // Get AST dateTime node
    string year = dateTime.child("year").text().empty() ? "" : dateTime.child("year").text().get();
    string month = dateTime.child("month").text().empty() ? "" : dateTime.child("month").text().get();
    string day = dateTime.child("day").text().empty() ? "" : dateTime.child("day").text().get();
    string hour = dateTime.child("hour").text().empty() ? "" : dateTime.child("hour").text().get();
    string minute = dateTime.child("minute").text().empty() ? "" : dateTime.child("minute").text().get();

    weather.year = year;
    weather.month = month;
    weather.day = day;
    weather.hour = hour;
    weather.minute = minute;

    // Get AST sunrise & sunset info
    for (pugi::xml_node sunriseNode = siteData.child("riseSet").find_child_by_attribute("dateTime", "name", "sunrise"); sunriseNode; sunriseNode = sunriseNode.next_sibling("dateTime")) {
        if (std::string(sunriseNode.attribute("zone").value()) == std::string("AST")) {
            weather.sunrise = std::string(sunriseNode.child("hour").text().get()) + std::string(":") + std::string(sunriseNode.child("minute").text().get());
            break;
        }
    }
    for (pugi::xml_node sunsetNode = siteData.child("riseSet").find_child_by_attribute("dateTime", "name", "sunset"); sunsetNode; sunsetNode = sunsetNode.next_sibling("dateTime")) {
        if (std::string(sunsetNode.attribute("zone").value()) == std::string("AST")) {
            weather.sunset = std::string(sunsetNode.child("hour").text().get()) + std::string(":") + std::string(sunsetNode.child("minute").text().get());
            break;
        }
    }

    // Get forecastGroup
    pugi::xml_node forecastGroup = siteData.child("forecastGroup");

    // Get Current Conditions
    pugi::xml_node currentConditions = siteData.child("currentConditions");
    double tempCur = currentConditions.child("temperature").text().as_double();
    double windChill = currentConditions.child("windChill").text().as_double();

    pugi::xml_node currentForecast = forecastGroup.child("forecast"); // The first forecast object should be Today or Tonight (depending on time of day).
    if (std::string(currentForecast.child("period").attribute("textForecastName").value()) == std::string("Today")) {
        // Set day text
        weather.currentConditions.day = currentForecast.child("period").text().get();

        weather.currentConditions.tempHigh = currentForecast.child("temperatures").find_child_by_attribute("temperature", "class", "high").text().as_double(-100);
        
        weather.currentConditions.pop = currentForecast.child("abbreviatedForecast").child("pop").text().as_int(-1);

        weather.currentConditions.textSummary = currentForecast.child("abbreviatedForecast").child("textSummary").text().get();

        // Extract the Type
        int iconCode = currentForecast.child("abbreviatedForecast").child("iconCode").text().as_int(-1);
        WeatherType type = extractWeatherType(iconCode, weather.currentConditions.textSummary);

        weather.currentConditions.type = type;

    } else {
        // If today's forecast data is already stored then keep it...
        if (weather.currentConditions.type == UNKNOWN) {
            // TODO IMPORTANT: Change the way this archived weather fetch works
            // If no forecast data was gathered for today,
            // fetch today's archived data to get this information.
            try {
                weather.currentConditions = FetchArchivedForecast();
            }
            catch(const std::exception& e) {
                std::cerr << e.what() << '\n';
                // TODO: Handle what to do if the archived forecast could not be successfully fetched.
            }
        }
    }

    // It's important to set these after, because they could have been overwritten if the else statement ran
    weather.currentConditions.tempCur = tempCur;
    weather.currentConditions.windChill = windChill;


    // Get Forecast for next 4 days
    int i = 0;
    for (pugi::xml_node forecast = forecastGroup.child("forecast"); forecast; forecast = forecast.next_sibling("forecast")) {
        // Exit if 4 have been retrieved
        if (i >= 4) { break; } 
        // Ignore any Nightly/Today Forecasts
        std::regex nightOrTodayRegex("(night)|(today)", std::regex_constants::icase);
        if (regex_search(forecast.child("period").attribute("textForecastName").value(), nightOrTodayRegex)) { continue; }
        
        // Set day text
        weather.forecast[i].day = forecast.child("period").text().get();

        // Set remaining info
        weather.forecast[i].tempHigh = forecast.child("temperatures").find_child_by_attribute("temperature", "class", "high").text().as_double(-100);

        int iconCode = forecast.child("abbreviatedForecast").child("iconCode").text().as_int(-1);
        int pop = forecast.child("abbreviatedForecast").child("pop").text().as_int(-1);
        string textSummary = forecast.child("abbreviatedForecast").child("textSummary").text().get();
        weather.forecast[i].type = extractWeatherType(iconCode, textSummary);
        weather.forecast[i].textSummary = textSummary;
        weather.forecast[i].pop = pop; 

        i++;
    }

    return;
}



// Draw Methods
void WeatherStationModule::DrawSeperatorLines() {
    rgb_matrix::DrawLine(off_screen_canvas, 0, 9, matrix_width, 9, clock_color);
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

void WeatherStationModule::DrawWeatherStationCanvas() {
    DrawSeperatorLines();
    //DrawCurrentDateTime();
    //DrawCurrentDayWeatherData();
    return;
}

void* WeatherStationModule::Main() {
    // TODO:
    // 1. Fetch weather data (and handle errors).
    //      - Make sure to only fetch the weather data every 15 minutes (or at specific times of the day).
    // 2. Draw canvas based on updated weather struct.

    ParseWeatherCanXMLData();
    // TODO: Handle errors

    DrawWeatherStationCanvas();

    // Update canvas to new time.
    t_mod->off_screen_canvas = off_screen_canvas;
    t_mod->update = true; // Set to true AFTER (to avoid RBW (Read Before Write) issues).



    while (t_mod->state != EXIT) {
        // TODO: Do nothing for now
    }

    // TODO: Need to enter
    pthread_exit(NULL);
}