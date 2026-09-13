#include "ScoreSystem.h"
#include <algorithm>


ScoreSystem::ScoreSystem()
{
}

ScoreSystem::~ScoreSystem()
{
}

void ScoreSystem::resetLevel()
{
    m_moves = 0;
    m_time = 0.0f;
}

void ScoreSystem::incrementMoves()
{
    m_moves++;
}

void ScoreSystem::updateTime(float deltaTime)
{
    m_time += deltaTime;
}

int ScoreSystem::calculateScore(int level, float wallDensity) const
{
    // Base score increases with level and difficulty (wall density)
    int baseScore = 1000 + (level * 150) + static_cast<int>(wallDensity * 500);

    // Penalties for moves and time
    int movePenalty = m_moves * 25;
    int timePenalty = static_cast<int>(m_time * 8.0f);

    int finalScore = baseScore - movePenalty - timePenalty;

    // Minimum score for completion
    return std::max(finalScore, 150);
}

int ScoreSystem::calculateStars(int level) const
{
    // Dynamic star targets: higher levels allow slightly more moves
    int target3Stars = 10 + (level / 2);
    int target2Stars = 18 + level;

    if (m_moves <= target3Stars)
    {
        return 3;
    }
    else if (m_moves <= target2Stars)
    {
        return 2;
    }
    return 1;
}
