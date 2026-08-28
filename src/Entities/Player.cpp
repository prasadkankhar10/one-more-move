#include "Player.h"
#include "World/Board.h"
#include "Core/Constants.h"
#include <cmath>

Player::Player()
{
}

Player::~Player()
{
}

void Player::reset(int x, int y)
{
    m_x = x;
    m_y = y;
    m_visualX = static_cast<float>(x);
    m_visualY = static_cast<float>(y);
    m_alive = true;
}

void Player::move(int dx, int dy, const Board& board)
{
    if (!m_alive) return;

    int nextX = m_x + dx;
    int nextY = m_y + dy;

    if (board.isValidPosition(nextX, nextY))
    {
        if (board.getTileType(nextX, nextY) != TileType::Wall)
        {
            m_x = nextX;
            m_y = nextY;
        }
    }
}

void Player::update(float deltaTime)
{
    if (!m_alive) return;

    //Snappy lerp speed
    const float lerpSpeed = 18.0f;
    
    m_visualX += (m_x - m_visualX) * lerpSpeed * deltaTime;
    m_visualY += (m_y - m_visualY) * lerpSpeed * deltaTime;

    // Snap to grid if extremely close to avoid perpetual updates
    if (std::abs(m_visualX - m_x) < 0.005f) m_visualX = static_cast<float>(m_x);
    if (std::abs(m_visualY - m_y) < 0.005f) m_visualY = static_cast<float>(m_y);
}

void Player::render(SDL_Renderer* renderer, float originX, float originY)
{
    if (!m_alive) return;

    // Calculate dynamic screen coordinates based on lerped visual grid positions
    SDL_FRect rect;
    rect.x = originX + m_visualX * (Constants::TILE_SIZE + Constants::GRID_SPACING);
    rect.y = originY + m_visualY * (Constants::TILE_SIZE + Constants::GRID_SPACING);
    
    // Player is slightly smaller than the tile for visual styling
    float padding = 4.0f;
    rect.x += padding;
    rect.y += padding;
    rect.w = static_cast<float>(Constants::TILE_SIZE) - (padding * 2.0f);
    rect.h = static_cast<float>(Constants::TILE_SIZE) - (padding * 2.0f);

    // Draw Player: Golden/Yellow solid square
    SDL_SetRenderDrawColor(renderer, 0xf6, 0xe0, 0x5e, 0xff); // Golden yellow
    SDL_RenderFillRect(renderer, &rect);

    // Add a small inner design (white center) to make it stand out
    SDL_FRect innerRect = rect;
    innerRect.x += 4.0f;
    innerRect.y += 4.0f;
    innerRect.w -= 8.0f;
    innerRect.h -= 8.0f;
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(renderer, &innerRect);
}
