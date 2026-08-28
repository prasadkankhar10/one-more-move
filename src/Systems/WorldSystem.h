#pragma once

#include "World/Board.h"
#include "Systems/AudioManager.h"
#include <random>

enum class WorldEventType
{
    WarningSpawn,
    TrapActivate,
    ShrinkBoard
};

class WorldSystem
{
public:
    WorldSystem();
    ~WorldSystem();

    // Runs after every player move. Updates hazards and triggers events.
    void updateWorld(Board& board, int playerX, int playerY, int exitX, int exitY, int moveCount, int level, AudioManager& audio);
    
    // Collapses 1 random walkable tile during sudden death
    void triggerSuddenDeathCollapse(Board& board, int playerX, int playerY, int exitX, int exitY, AudioManager& audio);

    void reset();

private:
    void triggerMajorEvent(Board& board, int playerX, int playerY, int exitX, int exitY, int level, AudioManager& audio);
    void spawnWarningTile(Board& board, int playerX, int playerY, int exitX, int exitY, AudioManager& audio);

    std::mt19937 m_rng;
    int m_shrinkOffset = 0; // Tracks how many times the board has shrunk
};
