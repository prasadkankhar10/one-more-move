#pragma once

#include <SDL3/SDL.h>
#include <string>

// Simple built-in 8x8 bitmap font (CGA/Spectrum style) to render text without external dependencies
namespace BitmapFont
{
    // 8x8 font data for ASCII chars 32 to 127
    extern const unsigned char font[96][8];

    // Renders a single character using merged horizontal runs for crisp rasterization
    void drawChar(SDL_Renderer* renderer, char c, float x, float y, float scale, SDL_Color color);

    // Renders a string of text with automatic high-contrast drop shadow
    void drawText(SDL_Renderer* renderer, const std::string& text, float x, float y, float scale, SDL_Color color);

    // Calculates width of text in screen pixels at the given scale
    float getTextWidth(const std::string& text, float scale);

    // Renders wrapped text bounded within maxWidth, breaking on whitespace. Returns final Y position.
    float drawTextWrapped(SDL_Renderer* renderer, const std::string& text, float x, float y, float maxWidth, float scale, SDL_Color color, float lineSpacing = 16.0f);

    // Calculates total rendered height of wrapped text
    float getTextHeightWrapped(const std::string& text, float maxWidth, float scale, float lineSpacing = 16.0f);
}
