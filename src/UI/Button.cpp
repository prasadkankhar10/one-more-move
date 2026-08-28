#include "Button.h"
#include "BitmapFont.h"

Button::Button()
{
}

Button::Button(float x, float y, float w, float h, const std::string& label)
    : m_label(label)
{
    m_rect.x = x;
    m_rect.y = y;
    m_rect.w = w;
    m_rect.h = h;
}

Button::~Button()
{
}

void Button::setPosition(float x, float y)
{
    m_rect.x = x;
    m_rect.y = y;
}

void Button::setSize(float w, float h)
{
    m_rect.w = w;
    m_rect.h = h;
}

bool Button::checkClick(float mx, float my) const
{
    return (mx >= m_rect.x && mx <= m_rect.x + m_rect.w &&
            my >= m_rect.y && my <= m_rect.y + m_rect.h);
}

void Button::render(SDL_Renderer* renderer, SDL_Color buttonColor, SDL_Color textColor)
{
    // Draw background rect
    SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
    SDL_RenderFillRect(renderer, &m_rect);

    // Draw thin highlight border
    SDL_FRect outlineRect = m_rect;
    SDL_SetRenderDrawColor(renderer, buttonColor.r + 30, buttonColor.g + 30, buttonColor.b + 30, 255);
    SDL_RenderRect(renderer, &outlineRect);

    // Calculate font scaling based on button size
    float scale = 1.5f;
    if (m_rect.h >= 60.0f) scale = 2.0f;

    // Center text label inside the button bounds
    float textWidth = m_label.length() * 8.0f * scale;
    float textHeight = 8.0f * scale;

    float tx = m_rect.x + (m_rect.w - textWidth) / 2.0f;
    float ty = m_rect.y + (m_rect.h - textHeight) / 2.0f;

    BitmapFont::drawText(renderer, m_label, tx, ty, scale, textColor);
}
