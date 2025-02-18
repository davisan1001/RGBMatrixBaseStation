#include "weather-station-module.hpp"

#include <curl/curl.h>
#include <cmath>
#include <regex>
#include <iostream>
#include "pugixml.hpp"

using namespace std;
using namespace Matrix;
using namespace WeatherStation;

string weatherTypeString[12] = { "SUN", "PARTLY_CLOUDY", "MOSTLY_CLOUDY", "LIGHT_FLURRIES", "SNOW", "CLOUD", "LIGHT_RAIN", "RAIN", "FREEZING_RAIN", "RAIN_SNOW", "THUNDERSHOWERS", "UNKNOWN" };

WeatherStationModule::WeatherStationModule(rgb_matrix::RGBMatrix* m) : MatrixModule(m) {
    // Setup default colors
    white_color = rgb_matrix::Color(255, 255, 255);
    seperator_color = rgb_matrix::Color(84, 84, 84);
    temp_cur_color = rgb_matrix::Color(255, 255, 255);
    temp_high_color = rgb_matrix::Color(255, 126, 0);
    windchill_color = rgb_matrix::Color(0, 183, 239);
    humidex_color = rgb_matrix::Color(255, 126, 0);
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
    const char* bdf_font_file = "../fonts/8x13_custom.bdf";

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

    // Set current network time
    SetCurrentNetworkTime();

    // Setup the Weather data storage struct
    weather = Weather();
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

// Time functions
void WeatherStationModule::SetCurrentNetworkTime() {
    // current date and time on the current system
    next_weather_update.tv_sec = time(NULL);
    next_weather_update.tv_nsec = 0;
    next_time.tv_sec = time(NULL);
    next_time.tv_nsec = 0;
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
    std::string response;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw runtime_error("Failed to initialize libcurl.");
        response = "";
    }

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

void WeatherStationModule::ParseWeatherCanXMLData() {
    pugi::xml_document doc;
    pugi::xml_parse_result result;

    try {
        result = doc.load_string(FetchData(weatherCanadaDatamartURL).c_str());
    }
    catch (const std::exception& e) {
        MatrixModule::LogError(e.what());
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
    weather.currentConditions.tempCur = currentConditions.child("temperature").text().as_string("--");
    
    // TODO: The below is untested (particularly the humidex part)
    if (currentConditions.child("windChill")) { // Get windchill (if it's winter and it exists)
        weather.currentConditions.feelsLike = currentConditions.child("windChill").text().as_string("--");
    } else if (currentConditions.child("humidex")) { // Get humidex (if it's summer and it exists)
        weather.currentConditions.feelsLike = currentConditions.child("humidex").text().as_string("--");
    }
    

    pugi::xml_node currentForecast = forecastGroup.child("forecast"); // The first forecast object should be Today or Tonight (depending on time of day).
    if (std::string(currentForecast.child("period").attribute("textForecastName").value()) == std::string("Today")) {
        // Set day text
        weather.currentConditions.day = currentForecast.child("period").text().get();

        weather.currentConditions.tempHigh = currentForecast.child("temperatures").find_child_by_attribute("temperature", "class", "high").text().as_string("--");

        weather.currentConditions.pop = currentForecast.child("abbreviatedForecast").child("pop").text().as_int(-1);

        weather.currentConditions.textSummary = currentForecast.child("abbreviatedForecast").child("textSummary").text().get();

        // Extract the Type
        int iconCode = currentForecast.child("abbreviatedForecast").child("iconCode").text().as_int(-1);
        WeatherType type = extractWeatherType(iconCode, weather.currentConditions.textSummary);

        weather.currentConditions.type = type;
    }
    else {
        // If today's forecast is NOT already stored...
        if (weather.currentConditions.type == UNKNOWN) {
            // If no forecast data was gathered for today,
            // fetch recent archived data to get this information.

            // TODO: Implement archived fetch
        }
    }


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
        weather.forecast[i].tempHigh = forecast.child("temperatures").find_child_by_attribute("temperature", "class", "high").text().as_string("--");

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
const uint8_t* WeatherStationModule::GetLargeImageByType(WeatherType type) {
    switch (type) {
    case SUN:
        return matrix_weather_images::large_sun_icon_option1;
        break;
    case PARTLY_CLOUDY:
    case MOSTLY_CLOUDY:
        return matrix_weather_images::large_sun_cloud_mix_icon_option1;
        break;
    case LIGHT_FLURRIES:
        return matrix_weather_images::large_light_flurries_icon;
        break;
    case SNOW:
        return matrix_weather_images::large_snow_icon_option1;
        break;
    case CLOUD:
        return matrix_weather_images::large_cloud_icon;
        break;
    case LIGHT_RAIN:
    case RAIN:
        return matrix_weather_images::large_rain_icon_option1;
        break;
    case FREEZING_RAIN:
        return matrix_weather_images::large_freezing_rain_icon_option1;
        break;
    case RAIN_SNOW:
        return matrix_weather_images::large_rain_snow_icon_option1;
        break;
    case THUNDERSHOWERS:
        return matrix_weather_images::large_thunder_showers_icon_option1;
        break;
    case UNKNOWN:
    default:
        return matrix_weather_images::large_error_icon;
        break;
    }
}

const uint8_t* WeatherStationModule::GetSmallImageByType(WeatherType type) {
    switch (type) {
    case SUN:
        return matrix_weather_images::small_sun_icon;
        break;
    case PARTLY_CLOUDY:
    case MOSTLY_CLOUDY:
        return matrix_weather_images::small_sun_cloud_mixed_icon_option1;
        break;
    case LIGHT_FLURRIES:
        return matrix_weather_images::small_light_flurries_icon;
        break;
    case SNOW:
        return matrix_weather_images::small_snow_icon;
        break;
    case CLOUD:
        return matrix_weather_images::small_cloud_icon;
        break;
    case LIGHT_RAIN:
    case RAIN:
        return matrix_weather_images::small_rain_icon;
        break;
    case FREEZING_RAIN:
        return matrix_weather_images::small_freezing_rain_icon;
        break;
    case RAIN_SNOW:
        return matrix_weather_images::small_snow_rain_icon_option1;
        break;
    case THUNDERSHOWERS:
        return matrix_weather_images::small_thunder_showers_icon_option1;
        break;
    case UNKNOWN:
    default:
        return matrix_weather_images::small_error_icon;
        break;
    }
}

void WeatherStationModule::DrawSeperatorLines() {
    rgb_matrix::DrawLine(off_screen_canvas, 0, 9, matrix_width, 9, seperator_color);
    rgb_matrix::DrawLine(off_screen_canvas, 0, 36, matrix_width, 36, seperator_color);
    rgb_matrix::DrawLine(off_screen_canvas, 47, 29, 47, 33, seperator_color);
    return;
}

void WeatherStationModule::DrawCurrentDateTime() {
    // Blank out the datetime section by drawing in black pixel values
    rgb_matrix::SetImage(off_screen_canvas, 0, 0, matrix_weather_images::datetime_erase_box,
        matrix_weather_images::datetime_erase_box_size,
        matrix_weather_images::datetime_erase_box_width,
        matrix_weather_images::datetime_erase_box_height, false);
    

    std::string month = std::to_string(local_time.tm_mon + 1);
    if (month.length() < 2) {
        month = "0" + month;
    }
    std::string day = std::to_string(local_time.tm_mday);

    std::string hour = (local_time.tm_hour % 12) == 0 ? std::to_string(12) : std::to_string(local_time.tm_hour % 12); // Convert 24 hour time to 12 hour time
    std::string minute = std::to_string(local_time.tm_min);
    if (hour.length() < 2) {
        hour = "0" + hour;
    }
    if (minute.length() < 2) {
        minute = "0" + minute;
    }

    std::string monthDayStr = month + "-" + day;
    std::string hourMinStr = hour + ":" + minute;

    std::string weekday;
    switch (local_time.tm_wday) {
    case 0:
        weekday = "SUN";
        break;
    case 1:
        weekday = "MON";
        break;
    case 2:
        weekday = "TUE";
        break;
    case 3:
        weekday = "WED";
        break;
    case 4:
        weekday = "THU";
        break;
    case 5:
        weekday = "FRI";
        break;
    case 6:
        weekday = "SAT";
        break;
    default:
        weekday = "---";
        break;
    }

    rgb_matrix::DrawText(
        off_screen_canvas, font, 2, 2 + font.baseline(), date_color, NULL, monthDayStr.c_str(), letter_spacing);
    rgb_matrix::DrawText(
        off_screen_canvas, font, 25, 2 + font.baseline(), white_color, NULL, hourMinStr.c_str(), letter_spacing);
    rgb_matrix::DrawText(
        off_screen_canvas, font, 51, 2 + font.baseline(), current_weekday_color, NULL, weekday.c_str(), letter_spacing);

    return;
}

void WeatherStationModule::DrawCurrentDayWeatherData() {
    // Draw weather icon
    rgb_matrix::SetImage(off_screen_canvas, 4, 13,
        GetLargeImageByType(weather.currentConditions.type),
        matrix_weather_images::large_weather_icon_size,
        matrix_weather_images::large_weather_icon_width,
        matrix_weather_images::large_weather_icon_height, false);

    // Draw current temp
    if (weather.currentConditions.tempCur != std::string("--")) {
        string currentTemp = std::to_string((int)std::round(stod(weather.currentConditions.tempCur))); // Round the value first
        currentTemp += "°";
        if (currentTemp.length() <= 3) { // If there are 2 characters or less (degree character takes 2 bytes)
            rgb_matrix::DrawText(
                off_screen_canvas, current_temp_font, 44, 13 + current_temp_font.baseline(), temp_cur_color, NULL, currentTemp.c_str(), letter_spacing);
        } else {
            rgb_matrix::DrawText(
                off_screen_canvas, current_temp_font, 40, 13 + current_temp_font.baseline(), temp_cur_color, NULL, currentTemp.c_str(), letter_spacing);
        }
    } else {
        rgb_matrix::DrawText(
            off_screen_canvas, current_temp_font, 44, 13 + current_temp_font.baseline(), temp_cur_color, NULL, weather.currentConditions.tempCur.c_str(), letter_spacing);
    }

    // Draw high temp
    if (weather.currentConditions.tempHigh != std::string("--")) {
        string highTemp = std::to_string((int)std::round(stod(weather.currentConditions.tempHigh))); // Round the value first
        highTemp += "°";
        if (highTemp.length() <= 3) { // If there are 2 characters or less (degree character takes 2 bytes)
            rgb_matrix::DrawText(
                off_screen_canvas, font, 40, 29 + font.baseline(), temp_high_color, NULL, highTemp.c_str(), letter_spacing);
        } else {
            rgb_matrix::DrawText(
                off_screen_canvas, font, 36, 29 + font.baseline(), temp_high_color, NULL, highTemp.c_str(), letter_spacing);
        }
    } else {
        rgb_matrix::DrawText(
            off_screen_canvas, font, 38, 29 + font.baseline(), temp_high_color, NULL, weather.currentConditions.tempHigh.c_str(), letter_spacing);
    }

    // Draw feelsLike (change colour depending if it's humidex or windchill)
    if (weather.currentConditions.feelsLike != std::string("--") && weather.currentConditions.tempCur != std::string("--")) {
        string feelsLike = std::to_string((int)std::round(stod(weather.currentConditions.feelsLike)));
        feelsLike += "°";
        if (stod(weather.currentConditions.feelsLike) > stod(weather.currentConditions.tempCur)) {
            rgb_matrix::DrawText(
                off_screen_canvas, font, 50, 29 + font.baseline(), humidex_color, NULL, feelsLike.c_str(), letter_spacing);
        } else {
            rgb_matrix::DrawText(
                off_screen_canvas, font, 50, 29 + font.baseline(), windchill_color, NULL, feelsLike.c_str(), letter_spacing);
        }
    } else {
        rgb_matrix::DrawText(
            off_screen_canvas, font, 50, 29 + font.baseline(), white_color, NULL, weather.currentConditions.feelsLike.c_str(), letter_spacing);
    }

    return;
}

void WeatherStationModule::DrawPredictedDailyForecastData() {
    int offset = 17;
    for (size_t i = 0; i < 4; i++) {
        // Draw weekday text
        std::string weekday = weather.forecast[i].day.substr(0, 2);
        std::transform(weekday.begin(), weekday.end(), weekday.begin(), ::toupper);
        rgb_matrix::DrawText(
            off_screen_canvas, font, 3 + (offset*i), 38 + font.baseline(), future_weekday_color, NULL, weekday.c_str(), letter_spacing);
        
        // Draw weather icon
        rgb_matrix::SetImage(off_screen_canvas, 3 + (offset*i), 44,
            GetSmallImageByType(weather.forecast[i].type),
            matrix_weather_images::small_weather_icon_size,
            matrix_weather_images::small_weather_icon_width,
            matrix_weather_images::small_weather_icon_height, false);

        // Draw temp high
        string highTemp = std::to_string((int)std::round(stod(weather.forecast[i].tempHigh))); // Round the value first
        highTemp += "°";
        if(highTemp.length() <= 3) { // If there are 2 characters or less (degree character takes 2 bytes)
            rgb_matrix::DrawText(
                off_screen_canvas, font, 5 + (offset*i), 53 + font.baseline(), temp_predicted_high_color, NULL, highTemp.c_str(), letter_spacing);
        } else {
            rgb_matrix::DrawText(
                off_screen_canvas, font, 3 + (offset*i), 53 + font.baseline(), temp_predicted_high_color, NULL, highTemp.c_str(), letter_spacing);
        }

        // Draw POP (if it exists)
        if(weather.forecast[i].pop > 0) {
            string pop = std::to_string(weather.forecast[i].pop);
            rgb_matrix::DrawText(
                off_screen_canvas, font, 2 + (offset*i), 59 + font.baseline(), temp_predicted_high_color, NULL, string(pop + "%").c_str(), letter_spacing);
        }
    }
    
    return;
}

void WeatherStationModule::DrawWeatherStationCanvas(bool dateTimeOnly) {
    if (dateTimeOnly) {
        DrawCurrentDateTime(); // Update the datetime only
    } else {
        off_screen_canvas->Fill(0, 0, 0);
        DrawSeperatorLines();
        DrawCurrentDateTime();
        DrawCurrentDayWeatherData();
        DrawPredictedDailyForecastData();
    }
    return;
}

rgb_matrix::FrameCanvas* WeatherStationModule::Update() {
    // Set readable local_time from next_time.tv_sec
    next_time.tv_sec = time(NULL);
    next_time.tv_nsec = 0;
    localtime_r(&next_time.tv_sec, &local_time);

    // If 20 minutes have passed, re-fetch & redraw weather data
    clock_gettime(CLOCK_REALTIME, &current_time);
    if (current_time.tv_sec >= next_weather_update.tv_sec) {
        next_weather_update.tv_sec = current_time.tv_sec + 60*20; // Wait another 20 minutes from now
        try {
            ParseWeatherCanXMLData();
        }
        catch(const std::exception& e) {
            MatrixModule::LogError(e.what());
        }
        DrawWeatherStationCanvas(false);
    } else {
        // Else redraw only the time
        // Draw the analog clock with the digital clock in the center.
        DrawWeatherStationCanvas(true);
    }

    // Wait to update time.
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, NULL);

    // Update the time seconds (to the next whole 15 second interval)
    //next_time.tv_sec = (current_time.tv_sec / 15 + 1) * 15;
    next_time.tv_sec = current_time.tv_sec + 1;

    return off_screen_canvas;
}
