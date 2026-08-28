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
