#include "Board.h"
#include "Core/Constants.h"
#include <iostream>
#include <queue>
#include <utility>

Board::Board()
{
}

Board::~Board()
{
}

void Board::init(int width, int height)
{
    m_width = width;
    m_height = height;

    // Resize grid and fill with empty tiles
    m_grid.assign(m_height, std::vector<Tile>(m_width, Tile{TileType::Empty, true}));

    // Calculate grid origin to center it on the screen
    float totalWidth = m_width * Constants::TILE_SIZE + (m_width - 1) * Constants::GRID_SPACING;
    float totalHeight = m_height * Constants::TILE_SIZE + (m_height - 1) * Constants::GRID_SPACING;

    m_originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
    m_originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;
}

bool Board::isValidPosition(int x, int y) const
{
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

TileType Board::getTileType(int x, int y) const
{
    if (isValidPosition(x, y))
    {
        return m_grid[y][x].type;
    }
    return TileType::Wall; // Treat out-of-bounds as wall/unreachable
}

void Board::setTileType(int x, int y, TileType type)
{
    if (isValidPosition(x, y))
    {
        m_grid[y][x].type = type;
    }
}

DebuffType Board::getDebuffType(int x, int y) const
{
    if (isValidPosition(x, y))
    {
        return m_grid[y][x].debuff;
    }
    return DebuffType::None;
}

void Board::setDebuffType(int x, int y, DebuffType debuff)
{
    if (isValidPosition(x, y))
    {
        m_grid[y][x].debuff = debuff;
    }
}

const Tile& Board::getTile(int x, int y) const
{
    static const Tile defaultWall{ TileType::Wall, true, DebuffType::None };
    if (isValidPosition(x, y))
    {
        return m_grid[y][x];
    }
    return defaultWall;
}

void Board::setTile(int x, int y, const Tile& tile)
{
    if (isValidPosition(x, y))
    {
        m_grid[y][x] = tile;
    }
}

SDL_FRect Board::getTileRect(int x, int y) const
{
    SDL_FRect rect;
    rect.x = m_originX + x * (Constants::TILE_SIZE + Constants::GRID_SPACING);
    rect.y = m_originY + y * (Constants::TILE_SIZE + Constants::GRID_SPACING);
    rect.w = static_cast<float>(Constants::TILE_SIZE);
    rect.h = static_cast<float>(Constants::TILE_SIZE);
    return rect;
}

void Board::render(SDL_Renderer* renderer, float offsetX, float offsetY)
{
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            SDL_FRect rect = getTileRect(x, y);
            rect.x += offsetX;
            rect.y += offsetY;

            // Draw a subtle border/background for grid cell
            // Fill color based on TileType
            switch (m_grid[y][x].type)
            {
                case TileType::Empty:
                    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff); // Dark grey/blue
                    break;
                case TileType::Wall:
                    SDL_SetRenderDrawColor(renderer, 0x4a, 0x55, 0x68, 0xff); // Lighter solid grey
                    break;
                case TileType::Exit:
                    SDL_SetRenderDrawColor(renderer, 0x48, 0xbb, 0x78, 0xff); // Green
                    break;
                case TileType::Danger:
                    SDL_SetRenderDrawColor(renderer, 0xf5, 0x65, 0x65, 0xff); // Red
                    break;
                case TileType::Trap:
                    SDL_SetRenderDrawColor(renderer, 0xed, 0x89, 0x36, 0xff); // Orange
                    break;
                case TileType::Curse:
                    SDL_SetRenderDrawColor(renderer, 0x80, 0x1f, 0x99, 0xff); // Deep purple
                    break;
                case TileType::Defuse:
                    SDL_SetRenderDrawColor(renderer, 0x23, 0x7a, 0x79, 0xff); // Deep cyan/teal
                    break;
            }

            SDL_RenderFillRect(renderer, &rect);

            // Draw a light outline around empty tiles to distinguish them
            if (m_grid[y][x].type == TileType::Empty)
            {
                SDL_SetRenderDrawColor(renderer, 0x3a, 0x45, 0x5c, 0xff);
                SDL_RenderRect(renderer, &rect);
            }
            // Draw special markings for Curse and Defuse
            else if (m_grid[y][x].type == TileType::Curse)
            {
                // Pulsing bright magenta inner diamond/box
                float pulse = 0.5f + 0.5f * std::sin(SDL_GetTicks() / 150.0f);
                SDL_FRect inner = rect;
                float pad = 8.0f - (pulse * 2.0f);
                inner.x += pad;
                inner.y += pad;
                inner.w -= pad * 2.0f;
                inner.h -= pad * 2.0f;
                SDL_SetRenderDrawColor(renderer, 0xd5, 0x3f, 0x8c, 0xff); // Bright magenta
                SDL_RenderFillRect(renderer, &inner);
            }
            else if (m_grid[y][x].type == TileType::Defuse)
            {
                // Bright cyan cross icon
                SDL_FRect centerBox = rect;
                centerBox.x += 6.0f;
                centerBox.y += 6.0f;
                centerBox.w -= 12.0f;
                centerBox.h -= 12.0f;
                SDL_SetRenderDrawColor(renderer, 0x38, 0xb2, 0xac, 0xff); // Bright cyan
                SDL_RenderFillRect(renderer, &centerBox);

                // White inner dot
                SDL_FRect dot = rect;
                dot.x += 14.0f;
                dot.y += 14.0f;
                dot.w -= 28.0f;
                dot.h -= 28.0f;
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderFillRect(renderer, &dot);
            }
        }
    }
}

bool Board::hasPath(int startX, int startY, int targetX, int targetY) const
{
    // If start or target is invalid, return false
    if (!isValidPosition(startX, startY) || !isValidPosition(targetX, targetY))
    {
        return false;
    }

    // BFS setup
    std::queue<std::pair<int, int>> q;
    std::vector<std::vector<bool>> visited(m_height, std::vector<bool>(m_width, false));

    q.push({startX, startY});
    visited[startY][startX] = true;

    // Helper directions (Up, Down, Left, Right)
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };

    while (!q.empty())
    {
        auto [cx, cy] = q.front();
        q.pop();

        if (cx == targetX && cy == targetY)
        {
            return true;
        }

        for (int i = 0; i < 4; ++i)
        {
            int nx = cx + dx[i];
            int nx_y = cy + dy[i]; // Wait, let's name it ny

            if (isValidPosition(nx, nx_y) && !visited[nx_y][nx])
            {
                TileType type = m_grid[nx_y][nx].type;
                bool isActive = m_grid[nx_y][nx].active;

                // BFS can traverse empty, exit, traps, and inactive danger (if any)
                // but cannot traverse Walls or active Danger tiles
                if (type != TileType::Wall && type != TileType::Danger && isActive)
                {
                    visited[nx_y][nx] = true;
                    q.push({nx, nx_y});
                }
            }
        }
    }

    return false;
}
