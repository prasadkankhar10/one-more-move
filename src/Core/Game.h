#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include "GameState.h"
#include "World/Board.h"
#include "Entities/Player.h"
#include "UI/HUD.h"
#include "Systems/AudioManager.h"
#include "Systems/WorldSystem.h"
#include "Systems/ScoreSystem.h"
#include "Systems/SaveSystem.h"
#include "World/LevelGenerator.h"

struct Particle
{
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    SDL_Color color;
    float size;
};

class Game
{
public:
    Game();
    ~Game();

    bool init();
    void run();
    void shutdown();

private:
    void processInput();
    void update(float deltaTime);
    void render();

    // Gameplay commands
    void startNewGame();
    void loadNextLevel();
    void handleMovement(int dx, int dy);
    void handleMouseClick(float mx, float my);
    void spawnExplosion(float x, float y, SDL_Color color);
    float getLevelTimeLimit() const;
    void triggerGameOver();
    void triggerLevelComplete();
    void triggerDebuff(DebuffType debuff);
    void reviseMap();
    void detonateBomb(int bx, int by);

    // SDL Core
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool m_running = false;
    GameState m_state = GameState::MainMenu;

    // Entities & Systems
    Board m_board;
    Player m_player;
    HUD m_hud;
    LevelGenerator m_generator;
    AudioManager m_audio;
    WorldSystem m_worldSystem;
    ScoreSystem m_scoreSystem;
    SaveData m_saveData;

    // Progression
    int m_currentLevel = 1;
    int m_currentSeed = 0;
    int m_accumulatedScore = 0;
    int m_levelBaseScore = 0;
    int m_spawnX = 1;
    int m_spawnY = 1;

    // Debuff State
    int m_reversedTurns = 0;
    std::string m_debuffMessage = "";
    float m_debuffMessageTimer = 0.0f;
    bool m_timedOut = false;
    float m_freezeTimer = 0.0f;

    // Touch & Swipe Controls
    float m_touchStartX = 0.0f;
    float m_touchStartY = 0.0f;
    bool m_isSwiping = false;

    // Turn control to prevent double-stepping on keyboard
    bool m_keyReleased = true;

    // Polish (Particles & Shake)
    std::vector<Particle> m_particles;
    float m_shakeTime = 0.0f;
    float m_shakeMagnitude = 0.0f;

    // Timing
    Uint64 m_lastTime = 0;

    std::string m_saveFilePath;
};
