#ifndef MATRIX_MODULE_H // include guard
#define MATRIX_MODULE_H

#include <string>

#include "graphics.h"
#include "led-matrix.h"
#include "pixel-mapper.h"

namespace Matrix {
    class MatrixModule {
    protected:
        static int matrix_width;
        static int matrix_height;

        rgb_matrix::FrameCanvas* off_screen_canvas;

        rgb_matrix::Font font;

        MatrixModule(rgb_matrix::RGBMatrix* m);
        MatrixModule(rgb_matrix::RGBMatrix* m, const char* bdf_font_file);

        // Error Logging Methods
        static void LogError(const std::string& errorMessage);

    public:
        // Initialize all necessary static member variables
        static void InitStaticMatrixVariables(rgb_matrix::RGBMatrix* m);

        virtual rgb_matrix::FrameCanvas* Update() = 0;

        virtual ~MatrixModule();
    };

} // namespace MatrixModule

#endif
