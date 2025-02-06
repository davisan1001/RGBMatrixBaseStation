#ifndef TEXT_TEST_MODULE_H // include guard
#define TEXT_TEST_MODULE_H

#include "matrix-module.hpp"

// Text displaying module for testing purposes
class TextTestModule : public MatrixModule {
    private:
        int text_start_x = 0;
        int text_start_y = 0;
        int letter_spacing = 0;
        rgb_matrix::Color text_color;
        std::string displayText;

        void ResetTextStartValues();

    public:
        TextTestModule(rgb_matrix::RGBMatrix *m);

        void setText(std::string text);

        void setColor(uint8_t rr, uint8_t gg, uint8_t bb);

        rgb_matrix::FrameCanvas *UpdateCanvas();
};

#endif