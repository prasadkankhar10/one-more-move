#pragma once

#include <SDL3/SDL.h>

class Board;

class Player
{
public:
    Player();
    ~Player();

    void reset(int x, int y);
    void move(int dx, int dy, const Board& board);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer, float originX, float originY);

    int getX() const { return m_x; }
    int getY() const { return m_y; }
    bool isAlive() const { return m_alive; }
    void setAlive(bool alive) { m_alive = alive; }

    float getVisualX() const { return m_visualX; }
    float getVisualY() const { return m_visualY; }

private:
    int m_x = 0;
    int m_y = 0;

    float m_visualX = 0.0f;
    float m_visualY = 0.0f;

    bool m_alive = true;
};
