#pragma once

class ScoreSystem
{
public:
    ScoreSystem();
    ~ScoreSystem();

    void resetLevel();
    void incrementMoves();
    void updateTime(float deltaTime);

    int calculateScore(int level, float wallDensity) const;
    int calculateStars(int level) const;

    int getMoves() const { return m_moves; }
    float getTime() const { return m_time; }

private:
    int m_moves = 0;
    float m_time = 0.0f;
};
