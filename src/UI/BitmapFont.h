#pragma once

#include <SDL3/SDL.h>
#include <string>

// Simple built-in 8x8 bitmap font (CGA/Spectrum style) to render text without external dependencies
namespace BitmapFont
{
    // 8x8 font data for ASCII chars 32 to 127
    extern const unsigned char font[96][8];

    // Renders a single character using SDL_RenderFillRects or SDL_RenderPoints for efficiency
    void drawChar(SDL_Renderer* renderer, char c, float x, float y, float scale, SDL_Color color);

    // Renders a string of text
    void drawText(SDL_Renderer* renderer, const std::string& text, float x, float y, float scale, SDL_Color color);
}
