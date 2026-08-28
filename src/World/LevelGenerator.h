#pragma once

#include "Board.h"
#include <random>

class LevelGenerator
{
public:
    LevelGenerator();
    ~LevelGenerator();

    // Generates a board, ensuring player start and exit positions have a valid path
    Board generate(int level, int seed, int& outPlayerX, int& outPlayerY, int& outExitX, int& outExitY);

private:
    float m_wallDensity = 0.15f;
    float m_dangerDensity = 0.05f;
};
