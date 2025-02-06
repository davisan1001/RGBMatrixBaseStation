#ifndef MATRIX_MODULE_H // include guard
#define MATRIX_MODULE_H

#include <iostream>

#include "graphics.h"
#include "led-matrix.h"
#include "pixel-mapper.h"

class MatrixModule {
	protected:
		static int matrix_width;
		static int matrix_height;

		rgb_matrix::FrameCanvas *off_screen_canvas; // TODO: Should each matrix module draw to the same off_screen_canvas? (i.e. make this static?)

		rgb_matrix::Font font;

		MatrixModule();
		MatrixModule(rgb_matrix::RGBMatrix *m);
        MatrixModule(rgb_matrix::RGBMatrix *m, const char *bdf_font_file);

	public:
		// Initialize all necessary static member variables
		static void InitStaticMatrixVariables(rgb_matrix::RGBMatrix *m);

		virtual rgb_matrix::FrameCanvas *UpdateCanvas() = 0;

		virtual ~MatrixModule();
};

#endif