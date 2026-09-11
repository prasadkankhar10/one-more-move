#include "LevelGenerator.h"
#include "Core/Constants.h"
#include <iostream>

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

        // Run BFS path verification
        if (board.hasPath(outPlayerX, outPlayerY, outExitX, outExitY))
        {
            board.setBiome(level);

            // Collect empty interior tiles (excluding player spawn and exit)
            std::vector<std::pair<int, int>> emptyTiles;
            for (int y = 1; y < rows - 1; ++y)
            {
                for (int x = 1; x < cols - 1; ++x)
                {
                    if (board.getTileType(x, y) == TileType::Empty &&
                        !(x == outPlayerX && y == outPlayerY) &&
                        !(x == outExitX && y == outExitY))
                    {
                        emptyTiles.push_back({x, y});
                    }
                }
            }

            std::shuffle(emptyTiles.begin(), emptyTiles.end(), rng);

            // 1. Spawn Coins (Level 2+, 1 to 3 coins)
            if (level >= 2 && !emptyTiles.empty())
            {
                int coinCount = std::min(static_cast<int>(emptyTiles.size()), 1 + (level % 3));
                for (int i = 0; i < coinCount && !emptyTiles.empty(); ++i)
                {
                    auto [cx, cy] = emptyTiles.back();
                    emptyTiles.pop_back();
                    board.setTileType(cx, cy, TileType::Coin);
                }
            }

            // 2. Spawn Curse/Debuff tiles (Level 2+, 1 to 3 tiles)
            if (level >= 2 && !emptyTiles.empty())
            {
                int curseCount = std::min(static_cast<int>(emptyTiles.size()), std::min(1 + (level / 4), 3));
                DebuffType debuffs[] = {
                    DebuffType::ReverseControls,
                    DebuffType::TeleportSpawn,
                    DebuffType::ReviseMap,
                    DebuffType::TimePenalty
                };

                for (int i = 0; i < curseCount && !emptyTiles.empty(); ++i)
                {
                    auto [cx, cy] = emptyTiles.back();
                    emptyTiles.pop_back();

                    board.setTileType(cx, cy, TileType::Curse);
                    DebuffType chosenDebuff = debuffs[rng() % 4];
                    board.setDebuffType(cx, cy, chosenDebuff);
                }

                // Spawn 1 Defuse tile on Level 3+
                if (level >= 3 && !emptyTiles.empty())
                {
                    auto [dx, dy] = emptyTiles.back();
                    emptyTiles.pop_back();
                    board.setTileType(dx, dy, TileType::Defuse);
                }
            }

            // 3. Spawn Crumbling floor tiles (Level 3+, 1 to 3 tiles)
            if (level >= 3 && !emptyTiles.empty())
            {
                int crumbCount = std::min(static_cast<int>(emptyTiles.size()), 1 + (level / 4));
                for (int i = 0; i < crumbCount && !emptyTiles.empty(); ++i)
                {
                    auto [cx, cy] = emptyTiles.back();
                    emptyTiles.pop_back();
                    board.setTileType(cx, cy, TileType::Crumbling);
                }
            }

            // 4. Spawn Key & Gate (Level 4+, 1 pair)
            if (level >= 4 && emptyTiles.size() >= 4)
            {
                // Find empty tile adjacent or close to exit for Gate
                int gateX = -1, gateY = -1;
                int keyX = -1, keyY = -1;

                // Pick key from back
                keyX = emptyTiles.back().first;
                keyY = emptyTiles.back().second;
                emptyTiles.pop_back();

                // Candidate gate
                gateX = emptyTiles.back().first;
                gateY = emptyTiles.back().second;
                emptyTiles.pop_back();

                board.setTileType(gateX, gateY, TileType::Gate);
                // Verify that player can reach the key while gate is locked
                if (board.hasPath(outPlayerX, outPlayerY, keyX, keyY))
                {
                    board.setTileType(keyX, keyY, TileType::Key);
                }
                else
                {
                    // Revert gate and key to empty if key is blocked
                    board.setTileType(gateX, gateY, TileType::Empty);
                    board.setTileType(keyX, keyY, TileType::Empty);
                }
            }

            // 5. Spawn Ice tiles (Level 5+, 2 to 4 slick tiles)
            if (level >= 5 && !emptyTiles.empty())
            {
                int iceCount = std::min(static_cast<int>(emptyTiles.size()), 2 + (level % 3));
                for (int i = 0; i < iceCount && !emptyTiles.empty(); ++i)
                {
                    auto [ix, iy] = emptyTiles.back();
                    emptyTiles.pop_back();
                    board.setTileType(ix, iy, TileType::Ice);
                }
            }

            // 6. Spawn Portals (Level 5+, 1 pair)
            if (level >= 5 && emptyTiles.size() >= 2 && (level % 2 == 1))
            {
                auto [p1x, p1y] = emptyTiles.back();
                emptyTiles.pop_back();
                auto [p2x, p2y] = emptyTiles.back();
                emptyTiles.pop_back();

                Tile t1{ TileType::Portal, true, DebuffType::None, p2x, p2y, 0 };
                Tile t2{ TileType::Portal, true, DebuffType::None, p1x, p1y, 0 };
                board.setTile(p1x, p1y, t1);
                board.setTile(p2x, p2y, t2);
            }

            // 7. Power-ups: Shield & TimeFreeze (Level 6+)
            if (level >= 6 && !emptyTiles.empty())
            {
                auto [sx, sy] = emptyTiles.back();
                emptyTiles.pop_back();
                board.setTileType(sx, sy, TileType::Shield);

                if (!emptyTiles.empty())
                {
                    auto [tx, ty] = emptyTiles.back();
                    emptyTiles.pop_back();
                    board.setTileType(tx, ty, TileType::TimeFreeze);
                }
            }

            // 8. Bomb (Level 7+, 1 bomb)
            if (level >= 7 && !emptyTiles.empty())
            {
                auto [bx, by] = emptyTiles.back();
                emptyTiles.pop_back();
                board.setTileType(bx, by, TileType::Bomb);
            }

            std::cout << "[Generator] Successfully generated solvable level " << level 
                      << " with seed " << seed << " in " << attempts << " attempts." << std::endl;
            return board;
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

    return board;
}
