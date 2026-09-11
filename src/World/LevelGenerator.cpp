#include "LevelGenerator.h"
#include "Core/Constants.h"
#include <iostream>
#include <set>
#include <algorithm>

LevelGenerator::LevelGenerator()
{
}

LevelGenerator::~LevelGenerator()
{
}

Board LevelGenerator::generate(int level, int seed, int& outPlayerX, int& outPlayerY, int& outExitX, int& outExitY)
{
    // Configure board settings based on level (difficulty)
    // Scale board size slowly with level
    int cols = Constants::DEFAULT_GRID_COLS;
    int rows = Constants::DEFAULT_GRID_ROWS;
    
    // Aggressive board size scaling
    if (level >= 4)
    {
        cols = 10;
        rows = 10;
    }
    if (level >= 7)
    {
        cols = 12;
        rows = 12;
    }

    // Wall and Danger density scaling
    m_wallDensity = 0.12f + std::min(level * 0.02f, 0.16f); // 12% to 28% max
    m_dangerDensity = 0.02f + std::min(level * 0.015f, 0.13f); // 2% to 15% max

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    Board board;
    int attempts = 0;
    const int maxAttempts = 150;

    // Loop until we find a solvable configuration
    while (attempts < maxAttempts)
    {
        attempts++;
        board.init(cols, rows);

        // Standard placement: spawn in bottom-left, exit in top-right
        outPlayerX = 1;
        outPlayerY = rows - 2;
        outExitX = cols - 2;
        outExitY = 1;

        // Fill boundaries and grid
        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                // Make outer borders walls
                if (x == 0 || x == cols - 1 || y == 0 || y == rows - 1)
                {
                    board.setTileType(x, y, TileType::Wall);
                    continue;
                }

                // Protect spawn and exit
                if (x == outPlayerX && y == outPlayerY)
                {
                    board.setTileType(x, y, TileType::Empty);
                    continue;
                }
                if (x == outExitX && y == outExitY)
                {
                    board.setTileType(x, y, TileType::Exit);
                    continue;
                }

                // Procedurally spawn walls and initial hazards
                float roll = dist(rng);
                if (roll < m_wallDensity)
                {
                    board.setTileType(x, y, TileType::Wall);
                }
                else if (level >= 3 && roll < (m_wallDensity + m_dangerDensity))
                {
                    board.setTileType(x, y, TileType::Danger);
                }
                else
                {
                    board.setTileType(x, y, TileType::Empty);
                }
            }
        }

        // 1. Initial BFS: Find the Golden Path from Spawn to Exit
        std::vector<std::pair<int, int>> goldenPath = board.findOptimalPath(outPlayerX, outPlayerY, outExitX, outExitY);
        if (!goldenPath.empty())
        {
            board.setBiome(level);

            std::set<std::pair<int, int>> goldenSet(goldenPath.begin(), goldenPath.end());

            // Separate empty interior tiles into off-path (hazards/puzzles) and on-path (safe)
            std::vector<std::pair<int, int>> offPathTiles;
            std::vector<std::pair<int, int>> onPathTiles;

            for (int y = 1; y < rows - 1; ++y)
            {
                for (int x = 1; x < cols - 1; ++x)
                {
                    if (board.getTileType(x, y) == TileType::Empty)
                    {
                        if (x == outPlayerX && y == outPlayerY) continue;
                        if (x == outExitX && y == outExitY) continue;

                        if (goldenSet.count({x, y}))
                        {
                            onPathTiles.push_back({x, y});
                        }
                        else
                        {
                            offPathTiles.push_back({x, y});
                        }
                    }
                }
            }

            std::shuffle(offPathTiles.begin(), offPathTiles.end(), rng);
            std::shuffle(onPathTiles.begin(), onPathTiles.end(), rng);

            // 1. Spawn Coins (Level 2+, 1 to 3 coins) - can appear on path or off path
            if (level >= 2)
            {
                int coinCount = 1 + (level % 3);
                for (int i = 0; i < coinCount; ++i)
                {
                    if (!offPathTiles.empty() && (i % 2 == 0 || onPathTiles.empty()))
                    {
                        auto [cx, cy] = offPathTiles.back();
                        offPathTiles.pop_back();
                        board.setTileType(cx, cy, TileType::Coin);
                    }
                    else if (!onPathTiles.empty())
                    {
                        auto [cx, cy] = onPathTiles.back();
                        onPathTiles.pop_back();
                        board.setTileType(cx, cy, TileType::Coin);
                    }
                }
            }

            // 2. Spawn Curse/Debuff tiles (Level 2+) - ONLY ON OFF-PATH TILES!
            // The Golden Path is never contaminated with mandatory curses!
            if (level >= 2 && !offPathTiles.empty())
            {
                int curseCount = std::min(static_cast<int>(offPathTiles.size()), std::min(1 + (level / 4), 3));
                DebuffType debuffs[] = {
                    DebuffType::ReverseControls,
                    DebuffType::TeleportSpawn,
                    DebuffType::ReviseMap,
                    DebuffType::TimePenalty
                };

                for (int i = 0; i < curseCount && !offPathTiles.empty(); ++i)
                {
                    auto [cx, cy] = offPathTiles.back();
                    offPathTiles.pop_back();

                    board.setTileType(cx, cy, TileType::Curse);
                    DebuffType chosenDebuff = debuffs[rng() % 4];
                    board.setDebuffType(cx, cy, chosenDebuff);
                }

                // Spawn 1 Defuse tile on Level 3+ (cures curse & disarms traps)
                if (level >= 3 && !offPathTiles.empty())
                {
                    auto [dx, dy] = offPathTiles.back();
                    offPathTiles.pop_back();
                    board.setTileType(dx, dy, TileType::Defuse);
                }
            }

            // 3. Spawn Crumbling floor tiles (Level 3+) - ONLY ON OFF-PATH TILES!
            if (level >= 3 && !offPathTiles.empty())
            {
                int crumbCount = std::min(static_cast<int>(offPathTiles.size()), 1 + (level / 4));
                for (int i = 0; i < crumbCount && !offPathTiles.empty(); ++i)
                {
                    auto [cx, cy] = offPathTiles.back();
                    offPathTiles.pop_back();
                    board.setTileType(cx, cy, TileType::Crumbling);
                }
            }

            // 4. Spawn Key & Gate (Level 4+, 1 pair)
            if (level >= 4 && offPathTiles.size() >= 2)
            {
                auto [kx, ky] = offPathTiles.back();
                offPathTiles.pop_back();
                auto [gx, gy] = offPathTiles.back();
                offPathTiles.pop_back();

                // Gate protects an off-path treasure chamber
                board.setTileType(gx, gy, TileType::Gate);
                board.setTileType(kx, ky, TileType::Key);
            }

            // 5. Spawn Ice tiles (Level 5+) - off-path slick tiles
            if (level >= 5 && !offPathTiles.empty())
            {
                int iceCount = std::min(static_cast<int>(offPathTiles.size()), 2 + (level % 3));
                for (int i = 0; i < iceCount && !offPathTiles.empty(); ++i)
                {
                    auto [ix, iy] = offPathTiles.back();
                    offPathTiles.pop_back();
                    board.setTileType(ix, iy, TileType::Ice);
                }
            }

            // 6. Spawn Portals (Level 5+, 1 pair on odd levels)
            if (level >= 5 && offPathTiles.size() >= 2 && (level % 2 == 1))
            {
                auto [p1x, p1y] = offPathTiles.back();
                offPathTiles.pop_back();
                auto [p2x, p2y] = offPathTiles.back();
                offPathTiles.pop_back();

                Tile t1{ TileType::Portal, true, DebuffType::None, p2x, p2y, 0 };
                Tile t2{ TileType::Portal, true, DebuffType::None, p1x, p1y, 0 };
                board.setTile(p1x, p1y, t1);
                board.setTile(p2x, p2y, t2);
            }

            // 7. Power-ups: Shield & TimeFreeze (Level 6+)
            if (level >= 6 && !offPathTiles.empty())
            {
                auto [sx, sy] = offPathTiles.back();
                offPathTiles.pop_back();
                board.setTileType(sx, sy, TileType::Shield);

                if (!offPathTiles.empty())
                {
                    auto [tx, ty] = offPathTiles.back();
                    offPathTiles.pop_back();
                    board.setTileType(tx, ty, TileType::TimeFreeze);
                }
            }

            // 8. Bomb (Level 7+, 1 bomb)
            if (level >= 7 && !offPathTiles.empty())
            {
                auto [bx, by] = offPathTiles.back();
                offPathTiles.pop_back();
                board.setTileType(bx, by, TileType::Bomb);
            }

            // Virtual Solver Verification:
            // Calculate final optimal path from player spawn to exit on the decorated board
            std::vector<std::pair<int, int>> finalOptimalPath = board.findOptimalPath(outPlayerX, outPlayerY, outExitX, outExitY);
            if (!finalOptimalPath.empty())
            {
                board.setOptimalPath(finalOptimalPath);
                std::cout << "[Generator] Successfully generated solvable level " << level 
                          << " with seed " << seed << " in " << attempts << " attempts. Optimal moves: " 
                          << finalOptimalPath.size() - 1 << std::endl;
                return board;
            }
        }
    }

    // Fallback: If we failed to generate a solvable level after max attempts,
    // generate a clean level without internal walls or hazards to guarantee solvability.
    std::cout << "[Generator] Fallback triggered! Generating clean level." << std::endl;
    board.init(cols, rows);
    outPlayerX = 1;
    outPlayerY = rows - 2;
    outExitX = cols - 2;
    outExitY = 1;

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            if (x == 0 || x == cols - 1 || y == 0 || y == rows - 1)
            {
                board.setTileType(x, y, TileType::Wall);
            }
            else if (x == outExitX && y == outExitY)
            {
                board.setTileType(x, y, TileType::Exit);
            }
            else
            {
                board.setTileType(x, y, TileType::Empty);
            }
        }
    }

    std::vector<std::pair<int, int>> fallbackPath = board.findOptimalPath(outPlayerX, outPlayerY, outExitX, outExitY);
    board.setOptimalPath(fallbackPath);
    return board;
}

