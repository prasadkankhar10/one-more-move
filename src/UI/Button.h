#pragma once

#include <SDL3/SDL.h>
#include <string>

class Button
{
public:
    Button();
    Button(float x, float y, float w, float h, const std::string& label);
    ~Button();

    void setPosition(float x, float y);
    void setSize(float w, float h);
    void setLabel(const std::string& label) { m_label = label; }

    bool checkClick(float mx, float my) const;
    void render(SDL_Renderer* renderer, SDL_Color buttonColor = { 74, 85, 104, 255 }, SDL_Color textColor = { 255, 255, 255, 255 });

    float getX() const { return m_rect.x; }
    float getY() const { return m_rect.y; }
    float getW() const { return m_rect.w; }
    float getH() const { return m_rect.h; }
    const SDL_FRect& getRect() const { return m_rect; }

private:
    SDL_FRect m_rect = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::string m_label = "";
};
