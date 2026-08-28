#include "HUD.h"
#include "BitmapFont.h"
#include "Core/Constants.h"
#include <iomanip>
#include <sstream>

HUD::HUD()
{
}

HUD::~HUD()
{
}

void HUD::init()
{
    // Initialize gameplay headers
    m_btnPause = Button(15.0f, 15.0f, 50.0f, 35.0f, "II");
    m_btnSound = Button(385.0f, 15.0f, 50.0f, 35.0f, "VOL");

    // Initialize direction pad (D-pad) for mobile touch
    float dpadCenterX = Constants::SCREEN_WIDTH / 2.0f;
    float dpadCenterY = 700.0f;
    float btnW = 65.0f;
    float btnH = 50.0f;
    float gap = 6.0f;

    m_btnUp    = Button(dpadCenterX - (btnW / 2.0f), dpadCenterY - btnH - gap, btnW, btnH, "UP");
    m_btnDown  = Button(dpadCenterX - (btnW / 2.0f), dpadCenterY + gap, btnW, btnH, "DOWN");
    m_btnLeft  = Button(dpadCenterX - (btnW / 2.0f) - btnW - gap, dpadCenterY - (btnH / 2.0f), btnW, btnH, "LEFT");
    m_btnRight = Button(dpadCenterX + (btnW / 2.0f) + gap, dpadCenterY - (btnH / 2.0f), btnW, btnH, "RIGHT");

    // Initialize overlay menu buttons
    m_btnPlay = Button(125.0f, 480.0f, 200.0f, 55.0f, "PLAY GAME");
    m_btnResume = Button(125.0f, 340.0f, 200.0f, 55.0f, "RESUME");
    m_btnNext = Button(125.0f, 440.0f, 200.0f, 55.0f, "NEXT LEVEL");
    m_btnRestart = Button(125.0f, 510.0f, 200.0f, 55.0f, "TRY AGAIN");
    m_btnMenu = Button(125.0f, 580.0f, 200.0f, 55.0f, "MAIN MENU");
}

void HUD::renderPlaying(SDL_Renderer* renderer, int level, int moves, int score, bool soundOn)
{
    // Draw background panel for stats
    SDL_FRect headerPanel = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), 65.0f };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xff); // Deeper dark panel
    SDL_RenderFillRect(renderer, &headerPanel);
    SDL_SetRenderDrawColor(renderer, 0x22, 0x27, 0x35, 0xff);
    SDL_RenderLine(renderer, 0.0f, 65.0f, static_cast<float>(Constants::SCREEN_WIDTH), 65.0f);

    // Renders Stats text
    std::stringstream ss;
    ss << "LEVEL: " << level;
    BitmapFont::drawText(renderer, ss.str(), 80.0f, 25.0f, 1.8f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "MOVES: " << moves;
    BitmapFont::drawText(renderer, ss.str(), 190.0f, 25.0f, 1.8f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "SCORE: " << score;
    BitmapFont::drawText(renderer, ss.str(), 295.0f, 25.0f, 1.8f, { 104, 219, 120, 255 }); // Green score

    // Render Buttons
    m_btnPause.render(renderer, { 45, 55, 72, 255 });
    m_btnSound.render(renderer, soundOn ? SDL_Color{ 45, 55, 72, 255 } : SDL_Color{ 110, 45, 45, 255 });

    // Render D-Pad
    m_btnUp.render(renderer, { 45, 55, 72, 255 });
    m_btnDown.render(renderer, { 45, 55, 72, 255 });
    m_btnLeft.render(renderer, { 45, 55, 72, 255 });
    m_btnRight.render(renderer, { 45, 55, 72, 255 });
}

void HUD::renderMainMenu(SDL_Renderer* renderer, int highScore)
{
    // Draw Title: ONE MORE MOVE
    BitmapFont::drawText(renderer, "ONE", 125.0f, 150.0f, 5.0f, { 246, 224, 94, 255 }); // Gold
    BitmapFont::drawText(renderer, "MORE", 125.0f, 210.0f, 5.0f, { 245, 101, 101, 255 }); // Red
    BitmapFont::drawText(renderer, "MOVE", 125.0f, 270.0f, 5.0f, { 255, 255, 255, 255 }); // White

    // Subtitle
    BitmapFont::drawText(renderer, "Every move changes the board.", 75.0f, 350.0f, 1.5f, { 160, 174, 192, 255 });

    // High Score
    std::stringstream ss;
    ss << "BEST SCORE: " << highScore;
    BitmapFont::drawText(renderer, ss.str(), 145.0f, 410.0f, 1.8f, { 104, 219, 120, 255 });

    // Render Play button
    m_btnPlay.render(renderer, { 72, 187, 120, 255 }); // Bright Green button
}

void HUD::renderPaused(SDL_Renderer* renderer)
{
    // Dark transparent overlay
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xbf); // 75% dark overlay
    SDL_RenderFillRect(renderer, &overlay);

    BitmapFont::drawText(renderer, "GAME PAUSED", 110.0f, 250.0f, 2.5f, { 255, 255, 255, 255 });

    m_btnResume.render(renderer, { 45, 55, 72, 255 });
    m_btnMenu.setPosition(125.0f, 420.0f);
    m_btnMenu.render(renderer, { 110, 45, 45, 255 }); // Dark Red
}

void HUD::renderGameOver(SDL_Renderer* renderer, int level, int score, int highScore)
{
    // Dark transparent overlay
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x1d, 0x13, 0x13, 0xdf); // Dark Red-tinted overlay
    SDL_RenderFillRect(renderer, &overlay);

    BitmapFont::drawText(renderer, "GAME OVER", 125.0f, 180.0f, 3.2f, { 245, 101, 101, 255 }); // Red

    std::stringstream lvlSS;
    lvlSS << "FAILED LEVEL " << level;
    float textWidth = lvlSS.str().length() * 8.0f * 1.8f;
    float tx = (Constants::SCREEN_WIDTH - textWidth) / 2.0f;
    BitmapFont::drawText(renderer, lvlSS.str(), tx, 240.0f, 1.8f, { 255, 255, 255, 255 });

    std::stringstream ss;
    ss << "Score: " << score;
    BitmapFont::drawText(renderer, ss.str(), 170.0f, 300.0f, 2.0f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "Best:  " << highScore;
    BitmapFont::drawText(renderer, ss.str(), 170.0f, 340.0f, 2.0f, { 246, 224, 94, 255 });

    m_btnRestart.setPosition(125.0f, 420.0f);
    m_btnRestart.render(renderer, { 72, 187, 120, 255 }); // Green
    m_btnMenu.setPosition(125.0f, 490.0f);
    m_btnMenu.render(renderer, { 45, 55, 72, 255 }); // Grey-blue
}

void HUD::renderLevelComplete(SDL_Renderer* renderer, int level, int score, int moves, float time, int stars, int highScore)
{
    // Dark transparent overlay
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x11, 0x1d, 0x14, 0xdf); // Green-tinted overlay
    SDL_RenderFillRect(renderer, &overlay);

    BitmapFont::drawText(renderer, "VICTORY!", 140.0f, 120.0f, 3.2f, { 72, 187, 120, 255 }); // Green

    std::stringstream lvlSS;
    lvlSS << "LEVEL " << level << " COMPLETE";
    float textWidth = lvlSS.str().length() * 8.0f * 1.8f;
    float tx = (Constants::SCREEN_WIDTH - textWidth) / 2.0f;
    BitmapFont::drawText(renderer, lvlSS.str(), tx, 175.0f, 1.8f, { 255, 255, 255, 255 });

    // Stars Rating
    // We draw stars using '*' character from our custom bitmap font
    std::string starsStr = "";
    for (int i = 0; i < 3; ++i)
    {
        starsStr += (i < stars) ? " * " : " - ";
    }
    BitmapFont::drawText(renderer, starsStr, 150.0f, 230.0f, 2.5f, { 246, 224, 94, 255 }); // Gold stars

    // Stats
    std::stringstream ss;
    ss << "Moves: " << moves;
    BitmapFont::drawText(renderer, ss.str(), 150.0f, 290.0f, 1.8f, { 255, 255, 255, 255 });

    ss.str("");
    ss << std::fixed << std::setprecision(1) << "Time:  " << time << "s";
    BitmapFont::drawText(renderer, ss.str(), 150.0f, 325.0f, 1.8f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "Score: " << score;
    BitmapFont::drawText(renderer, ss.str(), 150.0f, 360.0f, 2.0f, { 104, 219, 120, 255 });

    ss.str("");
    ss << "Best:  " << highScore;
    BitmapFont::drawText(renderer, ss.str(), 150.0f, 395.0f, 1.8f, { 246, 224, 94, 255 });

    m_btnNext.render(renderer, { 72, 187, 120, 255 }); // Green
    m_btnRestart.setPosition(125.0f, 510.0f);
    m_btnRestart.render(renderer, { 45, 55, 72, 255 }); // Grey-blue
    m_btnMenu.setPosition(125.0f, 580.0f);
    m_btnMenu.render(renderer, { 110, 45, 45, 255 }); // Red
}
