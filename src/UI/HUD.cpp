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
    m_btnPause = Button(15.0f, 12.0f, 42.0f, 32.0f, "II");
    m_btnSound = Button(393.0f, 12.0f, 42.0f, 32.0f, "VOL");
    m_btnControls = Button(15.0f, 48.0f, 80.0f, 26.0f, "BOTH");

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
    m_btnPlay = Button(85.0f, 390.0f, 280.0f, 55.0f, "PLAY GAME");
    m_btnExit = Button(85.0f, 530.0f, 280.0f, 45.0f, "EXIT GAME");
    m_btnResume = Button(115.0f, 300.0f, 220.0f, 50.0f, "RESUME");
    m_btnNext = Button(115.0f, 440.0f, 220.0f, 55.0f, "NEXT LEVEL");
    m_btnRestart = Button(115.0f, 510.0f, 220.0f, 50.0f, "TRY AGAIN");
    m_btnMenu = Button(115.0f, 580.0f, 220.0f, 50.0f, "MAIN MENU");
}

void HUD::renderPlaying(SDL_Renderer* renderer, int level, int moves, int score, bool soundOn, float timeLeft, float timeLimit, int controlMode, int reversedTurns, const std::string& debuffMsg)
{
    // Draw background panel for stats (82px height prevents any text collisions)
    SDL_FRect headerPanel = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), 82.0f };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xff); // Deeper dark panel
    SDL_RenderFillRect(renderer, &headerPanel);
    SDL_SetRenderDrawColor(renderer, 0x22, 0x27, 0x35, 0xff);
    SDL_RenderLine(renderer, 0.0f, 82.0f, static_cast<float>(Constants::SCREEN_WIDTH), 82.0f);

    // Row 1 (y = 18): Buttons + Level + Moves
    m_btnPause.render(renderer, { 45, 55, 72, 255 });
    m_btnSound.render(renderer, soundOn ? SDL_Color{ 45, 55, 72, 255 } : SDL_Color{ 110, 45, 45, 255 });

    std::stringstream ss;
    ss << "LVL: " << level;
    BitmapFont::drawText(renderer, ss.str(), 72.0f, 20.0f, 1.5f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "MOVES: " << moves;
    BitmapFont::drawText(renderer, ss.str(), 185.0f, 20.0f, 1.5f, { 255, 255, 255, 255 });

    // Row 2 (y = 52): Control mode toggle button + Score + Time bar
    std::string ctrlLabel = (controlMode == 0) ? "BOTH" : (controlMode == 1 ? "SWIPE" : "DPAD");
    m_btnControls.setPosition(15.0f, 48.0f);
    m_btnControls.setSize(75.0f, 26.0f);
    m_btnControls.setLabel(ctrlLabel);
    m_btnControls.render(renderer, { 35, 42, 58, 255 }, { 160, 174, 192, 255 });

    ss.str("");
    ss << "PTS: " << score;
    BitmapFont::drawText(renderer, ss.str(), 100.0f, 54.0f, 1.4f, { 104, 219, 120, 255 }); // Green score

    // Time text & progress bar
    if (timeLeft < 0.0f) timeLeft = 0.0f;
    char timeBuffer[16];
    snprintf(timeBuffer, sizeof(timeBuffer), "%.1fs", timeLeft);

    bool isLowTime = (timeLeft < 5.0f);
    SDL_Color timeColor = isLowTime ? SDL_Color{ 245, 101, 101, 255 } : SDL_Color{ 246, 224, 94, 255 };
    BitmapFont::drawText(renderer, timeBuffer, 235.0f, 54.0f, 1.4f, timeColor);

    // Time Progress Bar
    float barX = 295.0f;
    float barY = 56.0f;
    float barW = 90.0f;
    float barH = 10.0f;

    SDL_FRect bgBar = { barX, barY, barW, barH };
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderFillRect(renderer, &bgBar);

    float ratio = (timeLimit > 0.0f) ? (timeLeft / timeLimit) : 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    if (ratio < 0.0f) ratio = 0.0f;

    SDL_FRect fillBar = { barX, barY, barW * ratio, barH };
    if (ratio > 0.3f)
    {
        SDL_SetRenderDrawColor(renderer, 72, 187, 120, 255); // Green
    }
    else
    {
        Uint8 alpha = (SDL_GetTicks() % 250 < 125) ? 140 : 255;
        SDL_SetRenderDrawColor(renderer, 245, 101, 101, alpha); // Flashing red
    }
    SDL_RenderFillRect(renderer, &fillBar);

    // Debuff Alert Banners below header
    if (reversedTurns > 0)
    {
        SDL_FRect debuffBar = { 15.0f, 88.0f, static_cast<float>(Constants::SCREEN_WIDTH - 30), 24.0f };
        SDL_SetRenderDrawColor(renderer, 0x80, 0x1f, 0x99, 0xdf); // Purple
        SDL_RenderFillRect(renderer, &debuffBar);

        std::string revText = "! CONTROLS REVERSED: " + std::to_string(reversedTurns) + " !";
        float tw = revText.length() * 8.0f * 1.4f;
        float tx = (Constants::SCREEN_WIDTH - tw) / 2.0f;
        BitmapFont::drawText(renderer, revText, tx, 92.0f, 1.4f, { 255, 255, 255, 255 });
    }
    else if (!debuffMsg.empty())
    {
        SDL_FRect debuffBar = { 15.0f, 88.0f, static_cast<float>(Constants::SCREEN_WIDTH - 30), 24.0f };
        SDL_SetRenderDrawColor(renderer, 0xc5, 0x30, 0x30, 0xdf); // Alert Red
        SDL_RenderFillRect(renderer, &debuffBar);

        float tw = debuffMsg.length() * 8.0f * 1.4f;
        float tx = (Constants::SCREEN_WIDTH - tw) / 2.0f;
        BitmapFont::drawText(renderer, debuffMsg, tx, 92.0f, 1.4f, { 255, 255, 255, 255 });
    }

    // Render D-Pad ONLY if controlMode allows it (0: Both, 2: D-Pad Only)
    if (controlMode != 1)
    {
        m_btnUp.render(renderer, { 45, 55, 72, 255 });
        m_btnDown.render(renderer, { 45, 55, 72, 255 });
        m_btnLeft.render(renderer, { 45, 55, 72, 255 });
        m_btnRight.render(renderer, { 45, 55, 72, 255 });
    }
}

void HUD::renderMainMenu(SDL_Renderer* renderer, int highScore, int highestLevel, int controlMode)
{
    // Draw Title: ONE MORE MOVE with shadow
    std::string title = "ONE MORE MOVE";
    float scale = 3.0f;
    float titleWidth = title.length() * 8.0f * scale;
    float titleX = (Constants::SCREEN_WIDTH - titleWidth) / 2.0f;

    // Drop shadow
    BitmapFont::drawText(renderer, title, titleX + 3.0f, 153.0f, scale, { 0, 0, 0, 180 });
    // Title Gold
    BitmapFont::drawText(renderer, title, titleX, 150.0f, scale, { 246, 224, 94, 255 });

    // Subtitle
    std::string sub = "TACTICAL ESCAPE";
    float subWidth = sub.length() * 8.0f * 1.5f;
    float subX = (Constants::SCREEN_WIDTH - subWidth) / 2.0f;
    BitmapFont::drawText(renderer, sub, subX, 195.0f, 1.5f, { 160, 174, 192, 255 });

    // Stats Card
    SDL_FRect statCard = { 55.0f, 235.0f, 340.0f, 120.0f };
    SDL_SetRenderDrawColor(renderer, 0x16, 0x1a, 0x24, 0xff);
    SDL_RenderFillRect(renderer, &statCard);
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderRect(renderer, &statCard);

    std::stringstream ss;
    ss << "UNLOCKED LEVEL: " << highestLevel;
    float sw1 = ss.str().length() * 8.0f * 1.6f;
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw1) / 2.0f, 260.0f, 1.6f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "BEST SCORE: " << highScore;
    float sw2 = ss.str().length() * 8.0f * 1.8f;
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw2) / 2.0f, 300.0f, 1.8f, { 104, 219, 120, 255 });

    // Menu Buttons
    m_btnPlay.setPosition(85.0f, 390.0f);
    m_btnPlay.setSize(280.0f, 55.0f);
    m_btnPlay.render(renderer, { 72, 187, 120, 255 }); // Bright Green button

    std::string ctrlText = (controlMode == 0) ? "CONTROLS: BOTH" : (controlMode == 1 ? "CONTROLS: SWIPE" : "CONTROLS: DPAD");
    m_btnControls.setPosition(85.0f, 465.0f);
    m_btnControls.setSize(280.0f, 45.0f);
    m_btnControls.setLabel(ctrlText);
    m_btnControls.render(renderer, { 45, 55, 72, 255 }, { 255, 255, 255, 255 });

    m_btnExit.setPosition(85.0f, 530.0f);
    m_btnExit.setSize(280.0f, 45.0f);
    m_btnExit.setLabel("EXIT GAME");
    m_btnExit.render(renderer, { 150, 45, 45, 255 }, { 255, 255, 255, 255 }); // Crimson
}

void HUD::renderPaused(SDL_Renderer* renderer)
{
    // Dark transparent overlay
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xcf); // Dark overlay
    SDL_RenderFillRect(renderer, &overlay);

    BitmapFont::drawText(renderer, "GAME PAUSED", 110.0f, 230.0f, 2.5f, { 255, 255, 255, 255 });

    m_btnResume.setPosition(115.0f, 300.0f);
    m_btnResume.setSize(220.0f, 50.0f);
    m_btnResume.render(renderer, { 72, 187, 120, 255 });

    m_btnMenu.setPosition(115.0f, 370.0f);
    m_btnMenu.setSize(220.0f, 50.0f);
    m_btnMenu.render(renderer, { 45, 55, 72, 255 });

    m_btnExit.setPosition(115.0f, 440.0f);
    m_btnExit.setSize(220.0f, 50.0f);
    m_btnExit.render(renderer, { 150, 45, 45, 255 }); // Dark Red Exit
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
