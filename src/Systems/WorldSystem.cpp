#include "WorldSystem.h"
#include <iostream>

WorldSystem::WorldSystem()
{
    // Seed with a random value on startup
    std::random_device rd;
    m_rng.seed(rd());
}

WorldSystem::~WorldSystem()
{
}

void WorldSystem::reset()
{
    m_shrinkOffset = 0;
}

void WorldSystem::updateWorld(Board& board, int playerX, int playerY, int exitX, int exitY, int moveCount, int level, AudioManager& audio)
{
    bool hazardActivated = false;

    // 1. Activate warning tiles (Trap -> Danger)
    for (int y = 0; y < board.getHeight(); ++y)
    {
        for (int x = 0; x < board.getWidth(); ++x)
        {
            if (board.getTileType(x, y) == TileType::Trap)
            {
                board.setTileType(x, y, TileType::Danger);
                hazardActivated = true;
            }
        }
    }

    if (hazardActivated)
    {
        audio.playDestructionSound();
    }

    // 2. Exit Relocation Survival Constraint (Level 6+, every 12 moves)
    if (level >= 6 && moveCount > 0 && moveCount % 12 == 0)
    {
        std::vector<std::pair<int, int>> safeTiles;
        for (int y = 1; y < board.getHeight() - 1; ++y)
        {
            for (int x = 1; x < board.getWidth() - 1; ++x)
            {
                // Must be Empty, not current player, and at least 4 tiles away (Manhattan distance)
                if (board.getTileType(x, y) == TileType::Empty && 
                    !(x == playerX && y == playerY) &&
                    (std::abs(playerX - x) + std::abs(playerY - y) >= 4))
                {
                    safeTiles.push_back({x, y});
                }
            }
        }

        if (!safeTiles.empty())
        {
            std::shuffle(safeTiles.begin(), safeTiles.end(), m_rng);
            for (const auto& tile : safeTiles)
            {
                int newEx = tile.first;
                int newEy = tile.second;

                // Temporarily place Exit to check pathfinding solvability
                board.setTileType(newEx, newEy, TileType::Exit);
                
                // Clear old exit
                board.setTileType(exitX, exitY, TileType::Empty);

                bool pathExists = board.hasPath(playerX, playerY, newEx, newEy);

                if (pathExists)
                {
                    std::cout << "[WorldSystem] Exit relocated to (" << newEx << ", " << newEy << ")!" << std::endl;
                    audio.playWarningSound();
                    
                    // Exit successfully moved! Update parameters for major checks.
                    exitX = newEx;
                    exitY = newEy;
                    break;
                }
                else
                {
                    // Rollback if path blocked
                    board.setTileType(newEx, newEy, TileType::Empty);
                    board.setTileType(exitX, exitY, TileType::Exit);
                }
            }
        }
    }

    // 3. Dynamic edge collapsing frequency
    int majorInterval = 5;
    if (level >= 5) majorInterval = 4;
    if (level >= 9) majorInterval = 3;

    if (moveCount > 0 && moveCount % majorInterval == 0)
    {
        triggerMajorEvent(board, playerX, playerY, exitX, exitY, level, audio);
    }
    else
    {
        // On regular turns, spawn warning tiles (scale trap frequency with level)
        int warningsToSpawn = 1;
        if (level >= 4) warningsToSpawn = 2;
        if (level >= 7) warningsToSpawn = 3;

        for (int i = 0; i < warningsToSpawn; ++i)
        {
            spawnWarningTile(board, playerX, playerY, exitX, exitY, audio);
        }
    }
}

void WorldSystem::spawnWarningTile(Board& board, int playerX, int playerY, int exitX, int exitY, AudioManager& audio)
{
    std::vector<std::pair<int, int>> emptyTiles;

    // Collect all safe, empty tiles
    for (int y = 1; y < board.getHeight() - 1; ++y)
    {
        for (int x = 1; x < board.getWidth() - 1; ++x)
        {
            if (board.getTileType(x, y) == TileType::Empty)
            {
                // Protect current player and exit cells
                if ((x == playerX && y == playerY) || (x == exitX && y == exitY))
                {
                    continue;
                }
                emptyTiles.push_back({x, y});
            }
        }
    }

    if (emptyTiles.empty()) return;

    // Shuffle tiles list
    std::shuffle(emptyTiles.begin(), emptyTiles.end(), m_rng);

    // Try to find a tile that doesn't block the exit when made into a wall/danger
    for (const auto& tile : emptyTiles)
    {
        int tx = tile.first;
        int ty = tile.second;

        // Temporarily change to Danger to run the BFS validation (Fairness Check)
        board.setTileType(tx, ty, TileType::Danger);
        bool pathExists = board.hasPath(playerX, playerY, exitX, exitY);
        board.setTileType(tx, ty, TileType::Empty); // Restore

        if (pathExists)
        {
            // Set to Trap (Orange Warning tile)
            board.setTileType(tx, ty, TileType::Trap);
            audio.playWarningSound();
            return; // Success
        }
    }
}

void WorldSystem::triggerMajorEvent(Board& board, int playerX, int playerY, int exitX, int exitY, int level, AudioManager& audio)
{
    // Determine major event type
    // If level is high (>= 5), allow shrinking the board edges
    bool tryShrink = (level >= 5 && m_shrinkOffset < (board.getWidth() / 2 - 2));
    
    // Choose shrinking vs spawning multiple hazard traps
    if (tryShrink && (m_shrinkOffset == 0 || rand() % 2 == 0))
    {
        // 1. Board collapse event (Shrink board)
        int cols = board.getWidth();
        int rows = board.getHeight();
        int offset = m_shrinkOffset;

        // Verify if player and exit are outside the collapsing boundary ring
        bool playerSafe = (playerX > offset && playerX < cols - 1 - offset &&
                            playerY > offset && playerY < rows - 1 - offset);
        
        bool exitSafe = (exitX > offset && exitX < cols - 1 - offset &&
                          exitY > offset && exitY < rows - 1 - offset);

        if (playerSafe && exitSafe)
        {
            // Temporarily flag outer ring as walls to check if a path still exists
            std::vector<std::pair<int, int>> targetRing;
            for (int x = offset; x < cols - offset; ++x)
            {
                targetRing.push_back({x, offset});
                targetRing.push_back({x, rows - 1 - offset});
            }
            for (int y = offset + 1; y < rows - 1 - offset; ++y)
            {
                targetRing.push_back({offset, y});
                targetRing.push_back({cols - 1 - offset, y});
            }

            std::vector<TileType> originalTypes;
            for (const auto& p : targetRing)
            {
                originalTypes.push_back(board.getTileType(p.first, p.second));
                board.setTileType(p.first, p.second, TileType::Wall);
            }

            bool pathExists = board.hasPath(playerX, playerY, exitX, exitY);

            if (pathExists)
            {
                // Commitment: collapse the outer border ring permanently
                m_shrinkOffset++;
                audio.playDeathSound(); // Play dynamic rumble
                std::cout << "[WorldSystem] Board edges collapsed! Offset: " << m_shrinkOffset << std::endl;
                return;
            }
            else
            {
                // Rollback: path would be blocked, skip shrinking this turn
                for (size_t i = 0; i < targetRing.size(); ++i)
                {
                    board.setTileType(targetRing[i].first, targetRing[i].second, originalTypes[i]);
                }
            }
        }
    }

    // 2. Spawning multiple warning tiles (high-intensity event)
    int warningsCount = (level < 5) ? 2 : 3;
    std::cout << "[WorldSystem] Major Hazard Event: Spawning " << warningsCount << " warning traps!" << std::endl;
    
    for (int i = 0; i < warningsCount; ++i)
    {
        spawnWarningTile(board, playerX, playerY, exitX, exitY, audio);
    }
}

void WorldSystem::triggerSuddenDeathCollapse(Board& board, int playerX, int playerY, int exitX, int exitY, AudioManager& audio)
{
    std::vector<std::pair<int, int>> walkableTiles;
    for (int y = 1; y < board.getHeight() - 1; ++y)
    {
        for (int x = 1; x < board.getWidth() - 1; ++x)
        {
            if (board.getTileType(x, y) == TileType::Empty)
            {
                if (!(x == playerX && y == playerY) && !(x == exitX && y == exitY))
                {
                    walkableTiles.push_back({x, y});
                }
            }
        }
    }

    if (walkableTiles.empty()) return;

    std::shuffle(walkableTiles.begin(), walkableTiles.end(), m_rng);

    for (const auto& tile : walkableTiles)
    {
        int tx = tile.first;
        int ty = tile.second;

        // Try to collapse it to a Wall
        board.setTileType(tx, ty, TileType::Wall);
        bool pathExists = board.hasPath(playerX, playerY, exitX, exitY);

        if (pathExists)
        {
            std::cout << "[Sudden Death] Collapsed safe tile at (" << tx << ", " << ty << ")!" << std::endl;
            audio.playDestructionSound();
            return; // Success
        }
        else
        {
            // Rollback if path blocked
            board.setTileType(tx, ty, TileType::Empty);
        }
    }
}
