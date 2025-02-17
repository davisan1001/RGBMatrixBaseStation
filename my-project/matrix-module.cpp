#include "matrix-module.hpp"

using namespace Matrix;

int MatrixModule::matrix_width;
int MatrixModule::matrix_height;

MatrixModule::MatrixModule(t_module* t_modArg) {
    t_mod = t_modArg;

    // Setup default t_module values
    t_mod->status = LOADING;
    t_mod->update = false;
    t_mod->off_screen_canvas = nullptr;
}

MatrixModule::MatrixModule(t_module* t_modArg, rgb_matrix::RGBMatrix* m) : MatrixModule::MatrixModule(t_modArg) {
	// Setup font
	const char* bdf_font_file = "../fonts/tom-thumb_fixed_4x6.bdf"; // TODO: This should be setable for each matrix module.

	if (bdf_font_file == NULL) {
		std::string errMsg = std::string("Unrecognized font file\n");
		std::cerr << errMsg.c_str();
		throw std::invalid_argument(errMsg);
	}

	// Load font. This needs to be a filename with a bdf bitmap font.
	if (!font.LoadFont(bdf_font_file)) {
		std::string errMsg =
			std::string("Couldn't load font \'") + bdf_font_file + "\'\n";
		std::cerr << errMsg.c_str();
		throw std::invalid_argument(errMsg);
	}

	// Store a reference to a new off_screen_canvas
	//    (one for each module initialized)
	off_screen_canvas = m->CreateFrameCanvas();
}

MatrixModule::MatrixModule(t_module* t_modArg, rgb_matrix::RGBMatrix* m, const char* bdf_font_file) : MatrixModule::MatrixModule(t_modArg) {
	if (bdf_font_file == NULL) {
		std::string errMsg = std::string("Unrecognized font file\n");
		std::cerr << errMsg.c_str();
		throw std::invalid_argument(errMsg);
	}

    // Load font. This needs to be a filename with a bdf bitmap font.
	if (!font.LoadFont(bdf_font_file)) {
		std::string errMsg =
			std::string("Couldn't load font \'") + bdf_font_file + "\'\n";
		std::cerr << errMsg.c_str();
		throw std::invalid_argument(errMsg);
	}

    // Store a reference to a new off_screen_canvas
	//    (one for each module initialized)
	off_screen_canvas = m->CreateFrameCanvas();
}

MatrixModule::~MatrixModule() {}


// Static Method Definitions
void MatrixModule::InitStaticMatrixVariables(rgb_matrix::RGBMatrix* m) {
	MatrixModule::matrix_width = m->width();
	MatrixModule::matrix_height = m->height();
}

void MatrixModule::LogError(const std::string& errorMessage) {
    std::ofstream logFile("log.txt", std::ios::app); // Open in append mode
    if (logFile.is_open()) {
        // Get current time
        std::time_t now = std::time(nullptr);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        // Write timestamp and error message
        logFile << "[" << timeStr << "] " << errorMessage << std::endl;
        logFile.close();
    } else {
        std::cerr << "Error: Could not open log file!" << std::endl;
    }
}

void *MatrixModule::Run(void * context) {
    return ((MatrixModule*)context)->Main();
}
