#include "text-test-module.hpp"
#include <sstream>

void TextTestModule::ResetTextStartValues() {
    text_start_x = 0;
    text_start_y = 0;
  }

TextTestModule::TextTestModule(rgb_matrix::RGBMatrix *m) : MatrixModule(m) {
    // Set text color default to white
    text_color = rgb_matrix::Color(255, 255, 255);

    // Set default text
    displayText = "Hello World!\nTime: 12:45";
  }

void TextTestModule::setText(std::string text) {
    displayText = text;
}

void TextTestModule::setColor(uint8_t rr, uint8_t gg, uint8_t bb) {
    text_color = rgb_matrix::Color(rr, gg, bb);
}

rgb_matrix::FrameCanvas *TextTestModule::UpdateCanvas() {
    std::stringstream streamText(displayText);
    std::string line;
    while (std::getline(streamText, line)) {
      rgb_matrix::DrawText(off_screen_canvas, font, text_start_x,
                           text_start_y + font.baseline(), text_color, NULL,
                           line.c_str(), letter_spacing);

      text_start_y += font.height();
    }

    ResetTextStartValues();

    return off_screen_canvas;
}