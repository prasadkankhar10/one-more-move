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

void Board::setBiome(int level)
{
    if (level <= 3) m_biome = 0;
    else if (level <= 6) m_biome = 1;
    else if (level <= 9) m_biome = 2;
    else if (level <= 12) m_biome = 3;
    else if (level <= 15) m_biome = 4;
    else if (level <= 18) m_biome = 5;
    else if (level <= 21) m_biome = 6;
    else m_biome = 7;
}

std::string Board::getBiomeName() const
{
    switch (m_biome)
    {
        case 0: return "MIDNIGHT DUNGEON";
        case 1: return "MOSSY RUINS";
        case 2: return "OCEAN ABYSS";
        case 3: return "VOLCANIC FORGE";
        case 4: return "GLACIAL CAVERN";
        case 5: return "NEON CYBERPUNK";
        case 6: return "PHARAOH TOMB";
        case 7: return "COSMIC VOID";
        default: return "DUNGEON";
    }
}

void Board::render(SDL_Renderer* renderer, float offsetX, float offsetY)
{
    Uint32 ticks = SDL_GetTicks();
    float pulse = 0.5f + 0.5f * std::sin(ticks / 180.0f);

    // Biome base colors (Default 0: Midnight Dungeon)
    Uint8 emptyR = 0x2d, emptyG = 0x35, emptyB = 0x48;
    Uint8 wallR = 0x4a, wallG = 0x55, wallB = 0x68;
    Uint8 outlineR = 0x3a, outlineG = 0x45, outlineB = 0x5c;

    if (m_biome == 1) // Mossy Ruins
    {
        emptyR = 0x1e; emptyG = 0x33; emptyB = 0x24;
        wallR = 0x2d; wallG = 0x5a; wallB = 0x36;
        outlineR = 0x38; outlineG = 0x73; outlineB = 0x45;
    }
    else if (m_biome == 2) // Ocean Abyss
    {
        emptyR = 0x11; emptyG = 0x2b; emptyB = 0x38;
        wallR = 0x1b; wallG = 0x4f; wallB = 0x63;
        outlineR = 0x26; outlineG = 0x72; outlineB = 0x8a;
    }
    else if (m_biome == 3) // Volcanic Forge
    {
        emptyR = 0x38; emptyG = 0x1a; emptyB = 0x1a;
        wallR = 0x6b; wallG = 0x2d; wallB = 0x2d;
        outlineR = 0x8a; outlineG = 0x34; outlineB = 0x34;
    }
    else if (m_biome == 4) // Glacial Cavern
    {
        emptyR = 0x1e; emptyG = 0x33; emptyB = 0x4d;
        wallR = 0x32; wallG = 0x55; wallB = 0x7e;
        outlineR = 0x4a; outlineG = 0x7c; outlineB = 0xb5;
    }
    else if (m_biome == 5) // Neon Cyberpunk
    {
        emptyR = 0x1c; emptyG = 0x14; emptyB = 0x38;
        wallR = 0x3b; wallG = 0x1d; wallB = 0x6b;
        outlineR = 0x63; outlineG = 0x29; outlineB = 0xb0;
    }
    else if (m_biome == 6) // Pharaoh Tomb
    {
        emptyR = 0x38; emptyG = 0x2b; emptyB = 0x16;
        wallR = 0x63; wallG = 0x4d; wallB = 0x24;
        outlineR = 0x8c; outlineG = 0x6e; outlineB = 0x35;
    }
    else if (m_biome == 7) // Cosmic Void
    {
        emptyR = 0x12; emptyG = 0x0e; emptyB = 0x24;
        wallR = 0x36; wallG = 0x1d; wallB = 0x52;
        outlineR = 0x54; outlineG = 0x2b; outlineB = 0x78;
    }

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            SDL_FRect rect = getTileRect(x, y);
            rect.x += offsetX;
            rect.y += offsetY;

            const Tile& tile = m_grid[y][x];

            switch (tile.type)
            {
                case TileType::Empty:
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    SDL_SetRenderDrawColor(renderer, outlineR, outlineG, outlineB, 0xff);
                    SDL_RenderRect(renderer, &rect);
                    break;

                case TileType::Wall:
                    SDL_SetRenderDrawColor(renderer, wallR, wallG, wallB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // 3D Bevel highlight
                    SDL_SetRenderDrawColor(renderer, 
                        static_cast<Uint8>(std::min(255, wallR + 40)),
                        static_cast<Uint8>(std::min(255, wallG + 40)),
                        static_cast<Uint8>(std::min(255, wallB + 40)), 0xff);
                    SDL_RenderLine(renderer, rect.x, rect.y, rect.x + rect.w, rect.y);
                    SDL_RenderLine(renderer, rect.x, rect.y, rect.x, rect.y + rect.h);
                    break;

                case TileType::Exit:
                {
                    SDL_SetRenderDrawColor(renderer, 0x48, 0xbb, 0x78, 0xff); // Bright green
                    SDL_RenderFillRect(renderer, &rect);
                    // Pulsing exit portal center
                    SDL_FRect inner = rect;
                    float pad = 8.0f - (pulse * 3.0f);
                    inner.x += pad; inner.y += pad;
                    inner.w -= pad * 2.0f; inner.h -= pad * 2.0f;
                    SDL_SetRenderDrawColor(renderer, 0x9a, 0xec, 0xa3, 0xff);
                    SDL_RenderFillRect(renderer, &inner);
                    break;
                }

                case TileType::Danger:
                {
                    SDL_SetRenderDrawColor(renderer, 0xf5, 0x65, 0x65, 0xff); // Red
                    SDL_RenderFillRect(renderer, &rect);
                    // Inner warning square
                    SDL_FRect inner = rect;
                    inner.x += 10.0f; inner.y += 10.0f;
                    inner.w -= 20.0f; inner.h -= 20.0f;
                    SDL_SetRenderDrawColor(renderer, 0x74, 0x2a, 0x2a, 0xff);
                    SDL_RenderFillRect(renderer, &inner);
                    break;
                }

                case TileType::Trap:
                {
                    SDL_SetRenderDrawColor(renderer, 0xed, 0x89, 0x36, 0xff); // Orange
                    SDL_RenderFillRect(renderer, &rect);
                    break;
                }

                case TileType::Curse:
                {
                    SDL_SetRenderDrawColor(renderer, 0x80, 0x1f, 0x99, 0xff); // Deep purple
                    SDL_RenderFillRect(renderer, &rect);
                    // Pulsing magenta inner diamond
                    SDL_FRect inner = rect;
                    float pad = 8.0f - (pulse * 2.0f);
                    inner.x += pad; inner.y += pad;
                    inner.w -= pad * 2.0f; inner.h -= pad * 2.0f;
                    SDL_SetRenderDrawColor(renderer, 0xd5, 0x3f, 0x8c, 0xff);
                    SDL_RenderFillRect(renderer, &inner);
                    break;
                }

                case TileType::Defuse:
                {
                    SDL_SetRenderDrawColor(renderer, 0x23, 0x7a, 0x79, 0xff); // Deep teal
                    SDL_RenderFillRect(renderer, &rect);
                    // Bright cyan cross icon
                    SDL_FRect centerBox = rect;
                    centerBox.x += 6.0f; centerBox.y += 6.0f;
                    centerBox.w -= 12.0f; centerBox.h -= 12.0f;
                    SDL_SetRenderDrawColor(renderer, 0x38, 0xb2, 0xac, 0xff);
                    SDL_RenderFillRect(renderer, &centerBox);
                    // White inner dot
                    SDL_FRect dot = rect;
                    dot.x += 14.0f; dot.y += 14.0f;
                    dot.w -= 28.0f; dot.h -= 28.0f;
                    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                    SDL_RenderFillRect(renderer, &dot);
                    break;
                }

                case TileType::Ice:
                {
                    // Glossy slick ice blue
                    SDL_SetRenderDrawColor(renderer, 0x76, 0xe4, 0xf7, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Diagonal gleam lines
                    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xcc);
                    SDL_RenderLine(renderer, rect.x + 6.0f, rect.y + rect.h - 8.0f, rect.x + rect.w - 8.0f, rect.y + 6.0f);
                    SDL_RenderLine(renderer, rect.x + 14.0f, rect.y + rect.h - 6.0f, rect.x + rect.w - 6.0f, rect.y + 14.0f);
                    // Subtle border
                    SDL_SetRenderDrawColor(renderer, 0x0b, 0xc5, 0xea, 0xff);
                    SDL_RenderRect(renderer, &rect);
                    break;
                }

                case TileType::Crumbling:
                {
                    // Earthy cracked floor
                    SDL_SetRenderDrawColor(renderer, 0xd6, 0x9e, 0x2e, 0xff); // Amber ochre
                    SDL_RenderFillRect(renderer, &rect);
                    // Fissure / crack pattern
                    SDL_SetRenderDrawColor(renderer, 0x74, 0x42, 0x10, 0xff);
                    SDL_RenderLine(renderer, rect.x + 4.0f, rect.y + 6.0f, rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f);
                    SDL_RenderLine(renderer, rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f, rect.x + rect.w - 6.0f, rect.y + 8.0f);
                    SDL_RenderLine(renderer, rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f, rect.x + rect.w / 2.0f + 4.0f, rect.y + rect.h - 4.0f);
                    break;
                }

                case TileType::Pit:
                {
                    // Bottomless black pit
                    SDL_SetRenderDrawColor(renderer, 0x0a, 0x0a, 0x0c, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Dark red/purple edge
                    SDL_SetRenderDrawColor(renderer, 0x4a, 0x15, 0x25, 0xff);
                    SDL_RenderRect(renderer, &rect);
                    break;
                }

                case TileType::Portal:
                {
                    // Cosmic swirl
                    SDL_SetRenderDrawColor(renderer, 0x2d, 0x1b, 0x4e, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Pulsing swirling rings
                    SDL_FRect ring = rect;
                    float pPad = 6.0f + (pulse * 4.0f);
                    ring.x += pPad; ring.y += pPad;
                    ring.w -= pPad * 2.0f; ring.h -= pPad * 2.0f;
                    SDL_SetRenderDrawColor(renderer, 0xb7, 0x94, 0xf4, 0xff); // Neon violet
                    SDL_RenderFillRect(renderer, &ring);
                    // Core
                    SDL_FRect core = rect;
                    core.x += 16.0f; core.y += 16.0f;
                    core.w -= 32.0f; core.h -= 32.0f;
                    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                    SDL_RenderFillRect(renderer, &core);
                    break;
                }

                case TileType::Key:
                {
                    // Base empty
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Golden Key shape
                    SDL_SetRenderDrawColor(renderer, 0xfa, 0xcc, 0x15, 0xff); // Bright gold
                    // Key head (ring)
                    SDL_FRect head = { rect.x + 8.0f, rect.y + 10.0f, 12.0f, 12.0f };
                    SDL_RenderFillRect(renderer, &head);
                    // Key head hole
                    SDL_FRect hole = { rect.x + 11.0f, rect.y + 13.0f, 6.0f, 6.0f };
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &hole);
                    // Key stem & tooth
                    SDL_SetRenderDrawColor(renderer, 0xfa, 0xcc, 0x15, 0xff);
                    SDL_FRect stem = { rect.x + 20.0f, rect.y + 14.0f, 12.0f, 4.0f };
                    SDL_RenderFillRect(renderer, &stem);
                    SDL_FRect tooth = { rect.x + 28.0f, rect.y + 18.0f, 4.0f, 6.0f };
                    SDL_RenderFillRect(renderer, &tooth);
                    break;
                }

                case TileType::Gate:
                {
                    // Dark stone door
                    SDL_SetRenderDrawColor(renderer, 0x2d, 0x37, 0x48, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Iron bars
                    SDL_SetRenderDrawColor(renderer, 0x71, 0x80, 0x96, 0xff);
                    for (float bx = 6.0f; bx < rect.w - 4.0f; bx += 8.0f)
                    {
                        SDL_RenderLine(renderer, rect.x + bx, rect.y + 4.0f, rect.x + bx, rect.y + rect.h - 4.0f);
                    }
                    // Golden Padlock in center
                    SDL_FRect lock = { rect.x + (rect.w - 14.0f) / 2.0f, rect.y + (rect.h - 12.0f) / 2.0f, 14.0f, 12.0f };
                    SDL_SetRenderDrawColor(renderer, 0xfa, 0xcc, 0x15, 0xff);
                    SDL_RenderFillRect(renderer, &lock);
                    break;
                }

                case TileType::Bomb:
                {
                    // Base empty
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Round bomb body
                    SDL_FRect bombBody = { rect.x + 10.0f, rect.y + 12.0f, rect.w - 20.0f, rect.h - 20.0f };
                    SDL_SetRenderDrawColor(renderer, 0x1a, 0x20, 0x2c, 0xff);
                    SDL_RenderFillRect(renderer, &bombBody);
                    // Fuse cap & burning spark
                    SDL_FRect cap = { rect.x + rect.w / 2.0f - 3.0f, rect.y + 8.0f, 6.0f, 4.0f };
                    SDL_SetRenderDrawColor(renderer, 0x71, 0x80, 0x96, 0xff);
                    SDL_RenderFillRect(renderer, &cap);
                    // Spark
                    SDL_FRect spark = { rect.x + rect.w / 2.0f - 2.0f, rect.y + 4.0f, 4.0f, 4.0f };
                    SDL_SetRenderDrawColor(renderer, (ticks % 200 < 100) ? 0xff : 0xed, 0x89, 0x36, 0xff);
                    SDL_RenderFillRect(renderer, &spark);
                    break;
                }

                case TileType::Shield:
                {
                    // Base empty
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Pulsing shield emblem
                    SDL_FRect sRect = rect;
                    sRect.x += 8.0f; sRect.y += 6.0f;
                    sRect.w -= 16.0f; sRect.h -= 12.0f;
                    SDL_SetRenderDrawColor(renderer, 0x31, 0x82, 0xce, 0xff); // Deep azure
                    SDL_RenderFillRect(renderer, &sRect);
                    // Inner crest
                    SDL_FRect crest = sRect;
                    crest.x += 4.0f; crest.y += 4.0f;
                    crest.w -= 8.0f; crest.h -= 8.0f;
                    SDL_SetRenderDrawColor(renderer, 0x90, 0xcd, 0xf4, 0xff); // Light cyan
                    SDL_RenderFillRect(renderer, &crest);
                    break;
                }

                case TileType::TimeFreeze:
                {
                    // Base empty
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Hourglass shape
                    SDL_FRect hOuter = { rect.x + 10.0f, rect.y + 6.0f, rect.w - 20.0f, rect.h - 12.0f };
                    SDL_SetRenderDrawColor(renderer, 0x00, 0xb5, 0xd8, 0xff); // Cyan clock
                    SDL_RenderRect(renderer, &hOuter);
                    // Inner sand/time diamond
                    SDL_FRect hInner = { rect.x + 14.0f, rect.y + 10.0f, rect.w - 28.0f, rect.h - 20.0f };
                    SDL_SetRenderDrawColor(renderer, 0xe6, 0xff, 0xfa, 0xff);
                    SDL_RenderFillRect(renderer, &hInner);
                    break;
                }

                case TileType::Coin:
                {
                    // Base empty
                    SDL_SetRenderDrawColor(renderer, emptyR, emptyG, emptyB, 0xff);
                    SDL_RenderFillRect(renderer, &rect);
                    // Gold Coin disk
                    SDL_FRect coinDisk = { rect.x + 10.0f, rect.y + 10.0f, rect.w - 20.0f, rect.h - 20.0f };
                    SDL_SetRenderDrawColor(renderer, 0xd6, 0x9e, 0x2e, 0xff); // Dark gold edge
                    SDL_RenderFillRect(renderer, &coinDisk);
                    SDL_FRect coinInner = { rect.x + 13.0f, rect.y + 13.0f, rect.w - 26.0f, rect.h - 26.0f };
                    SDL_SetRenderDrawColor(renderer, 0xfa, 0xcc, 0x15, 0xff); // Shiny center
                    SDL_RenderFillRect(renderer, &coinInner);
                    break;
                }
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

        // Check if current tile is a portal to teleport
        const Tile& curTile = m_grid[cy][cx];
        if (curTile.type == TileType::Portal && isValidPosition(curTile.portalTargetX, curTile.portalTargetY))
        {
            int px = curTile.portalTargetX;
            int py = curTile.portalTargetY;
            if (!visited[py][px])
            {
                visited[py][px] = true;
                q.push({px, py});
            }
        }

        for (int i = 0; i < 4; ++i)
        {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (isValidPosition(nx, ny) && !visited[ny][nx])
            {
                TileType type = m_grid[ny][nx].type;
                bool isImpassable = (type == TileType::Wall || type == TileType::Danger || 
                                     type == TileType::Pit || type == TileType::Gate);

                if (!isImpassable)
                {
                    visited[ny][nx] = true;
                    q.push({nx, ny});
                }
            }
        }
    }

    return false;
}

