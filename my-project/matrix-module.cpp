#include "matrix-module.hpp"

int MatrixModule::matrix_width;
int MatrixModule::matrix_height;

MatrixModule::MatrixModule() {}

MatrixModule::MatrixModule(rgb_matrix::RGBMatrix* m) {
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

MatrixModule::MatrixModule(rgb_matrix::RGBMatrix* m, const char* bdf_font_file) {
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

void MatrixModule::InitStaticMatrixVariables(rgb_matrix::RGBMatrix* m) {
	MatrixModule::matrix_width = m->width();
	MatrixModule::matrix_height = m->height();
}
