#pragma once

#include <SDL3/SDL.h>
#include "Button.h"

class HUD
{
public:
    HUD();
    ~HUD();

    void init();
    
    // Renders HUD during playing state (top stats + bottom D-pad)
    void renderPlaying(SDL_Renderer* renderer, int level, int moves, int score, bool soundOn, float timeLeft, float timeLimit, int controlMode, int reversedTurns, const std::string& debuffMsg, bool hasShield = false, bool hasKey = false, float freezeTime = 0.0f, const std::string& biomeName = "");

    // Overlay Screen Renderers
    void renderMainMenu(SDL_Renderer* renderer, int highScore, int highestLevel, int controlMode);
    void renderPaused(SDL_Renderer* renderer);
    void renderGameOver(SDL_Renderer* renderer, int level, int score, int highScore, bool timedOut = false);
    void renderLevelComplete(SDL_Renderer* renderer, int level, int score, int moves, float time, int stars, int highScore);
    void renderInfo(SDL_Renderer* renderer, int currentTab);
    void renderPathPreview(SDL_Renderer* renderer, int pathMoves);

    // Getters for touch buttons (so Game class can check tap inputs)
    const Button& getBtnUp() const { return m_btnUp; }
    const Button& getBtnDown() const { return m_btnDown; }
    const Button& getBtnLeft() const { return m_btnLeft; }
    const Button& getBtnRight() const { return m_btnRight; }
    const Button& getBtnSound() const { return m_btnSound; }
    const Button& getBtnPause() const { return m_btnPause; }
    const Button& getBtnControls() const { return m_btnControls; }
    const Button& getBtnExit() const { return m_btnExit; }

    // Screen-specific buttons
    Button m_btnPlay;
    Button m_btnResume;
    Button m_btnRestart;
    Button m_btnShowPath; // Reveal optimal winning path on game over
    Button m_btnNext;
    Button m_btnMenu; // Back to menu button used on multiple screens
    Button m_btnExit; // Clean app quit button
    Button m_btnControls; // Control mode toggle
    Button m_btnGiveUp; // Concede current level
    Button m_btnRestartRun; // Restart entire run from Level 1
    Button m_btnInfo; // Open How To Play / Info
    Button m_btnBack; // Back button on info screen


    // Info tabs
    Button m_btnTabPlay;
    Button m_btnTabTiles;
    Button m_btnTabDev;

private:
    // Directional pad buttons
    Button m_btnUp;
    Button m_btnDown;
    Button m_btnLeft;
    Button m_btnRight;

    // Header buttons
    Button m_btnSound;
    Button m_btnPause;
};
