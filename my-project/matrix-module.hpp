#ifndef MATRIX_MODULE_H // include guard
#define MATRIX_MODULE_H

#include <iostream>

#include "graphics.h"
#include "led-matrix.h"
#include "pixel-mapper.h"

typedef struct {
    bool update;
    rgb_matrix::FrameCanvas *off_screen_canvas;
} t_module;

class MatrixModule {
	protected:
        t_module *t_mod;
        
		static int matrix_width;
		static int matrix_height;

		rgb_matrix::FrameCanvas *off_screen_canvas; // TODO: Should each matrix module draw to the same off_screen_canvas? (i.e. make this static?)

		rgb_matrix::Font font;

		MatrixModule(t_module* t_modArg);
		MatrixModule(t_module* t_modArg, rgb_matrix::RGBMatrix *m);
        MatrixModule(t_module* t_modArg, rgb_matrix::RGBMatrix *m, const char *bdf_font_file);

	public:
		// Initialize all necessary static member variables
		static void InitStaticMatrixVariables(rgb_matrix::RGBMatrix *m);

        static void *StartThreadRun(void * context);

		virtual void *Run() = 0;

		virtual ~MatrixModule();
};

#endif