#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "Tile.h"

class Board
{
public:
    Board();
    ~Board();

    void init(int width, int height);
    void render(SDL_Renderer* renderer, float offsetX = 0.0f, float offsetY = 0.0f);

    bool isValidPosition(int x, int y) const;
    TileType getTileType(int x, int y) const;
    void setTileType(int x, int y, TileType type);
    bool hasPath(int startX, int startY, int targetX, int targetY) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // Helpers to get screen coordinates for visual interpolation
    SDL_FRect getTileRect(int x, int y) const;

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<std::vector<Tile>> m_grid;

    // Grid screen positions
    float m_originX = 0.0f;
    float m_originY = 0.0f;
};
