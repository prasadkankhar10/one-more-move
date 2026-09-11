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
    m_btnPause = Button(10.0f, 12.0f, 36.0f, 30.0f, "II");
    m_btnGiveUp = Button(50.0f, 12.0f, 54.0f, 30.0f, "GIVE");
    m_btnRestartRun = Button(108.0f, 12.0f, 54.0f, 30.0f, "LVL 1");
    m_btnSound = Button(404.0f, 12.0f, 36.0f, 30.0f, "VOL");
    m_btnControls = Button(10.0f, 48.0f, 75.0f, 26.0f, "BOTH");

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
    m_btnPlay = Button(85.0f, 365.0f, 280.0f, 50.0f, "PLAY GAME");
    m_btnInfo = Button(85.0f, 425.0f, 280.0f, 45.0f, "HOW TO PLAY & INFO");
    m_btnExit = Button(85.0f, 535.0f, 280.0f, 45.0f, "EXIT GAME");
    m_btnResume = Button(85.0f, 240.0f, 280.0f, 48.0f, "RESUME");
    m_btnNext = Button(115.0f, 440.0f, 220.0f, 55.0f, "NEXT LEVEL");
    m_btnRestart = Button(115.0f, 510.0f, 220.0f, 50.0f, "TRY AGAIN");
    m_btnMenu = Button(85.0f, 420.0f, 280.0f, 48.0f, "MAIN MENU");

    // Info screen tab buttons
    m_btnTabPlay = Button(15.0f, 65.0f, 135.0f, 36.0f, "HOW TO PLAY");
    m_btnTabTiles = Button(157.0f, 65.0f, 135.0f, 36.0f, "ALL TILES");
    m_btnTabDev = Button(299.0f, 65.0f, 135.0f, 36.0f, "DEVELOPER");
    m_btnBack = Button(115.0f, 735.0f, 220.0f, 45.0f, "BACK TO MENU");
}

void HUD::renderPlaying(SDL_Renderer* renderer, int level, int moves, int score, bool soundOn, float timeLeft, float timeLimit, int controlMode, int reversedTurns, const std::string& debuffMsg, bool hasShield, bool hasKey, float freezeTime, const std::string& biomeName)
{
    // Draw background panel for stats (82px height prevents any text collisions)
    SDL_FRect headerPanel = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), 82.0f };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xff); // Deeper dark panel
    SDL_RenderFillRect(renderer, &headerPanel);
    SDL_SetRenderDrawColor(renderer, 0x22, 0x27, 0x35, 0xff);
    SDL_RenderLine(renderer, 0.0f, 82.0f, static_cast<float>(Constants::SCREEN_WIDTH), 82.0f);

    // Row 1 (y = 12): [II] [GIVE] [LVL 1] ... L:X ... M:X ... [SHLD] [KEY] ... [VOL]
    m_btnPause.setPosition(10.0f, 12.0f);
    m_btnPause.setSize(36.0f, 30.0f);
    m_btnPause.render(renderer, { 45, 55, 72, 255 });

    m_btnGiveUp.setPosition(50.0f, 12.0f);
    m_btnGiveUp.setSize(54.0f, 30.0f);
    m_btnGiveUp.setLabel("GIVE");
    m_btnGiveUp.render(renderer, { 140, 45, 45, 255 }, { 255, 200, 200, 255 });

    m_btnRestartRun.setPosition(108.0f, 12.0f);
    m_btnRestartRun.setSize(54.0f, 30.0f);
    m_btnRestartRun.setLabel("LVL 1");
    m_btnRestartRun.render(renderer, { 160, 90, 30, 255 }, { 255, 230, 180, 255 });

    std::stringstream ss;
    ss << "L:" << level;
    BitmapFont::drawText(renderer, ss.str(), 168.0f, 19.0f, 1.5f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "M:" << moves;
    BitmapFont::drawText(renderer, ss.str(), 218.0f, 19.0f, 1.5f, { 255, 255, 255, 255 });

    // Inventory indicators in Row 1
    float badgeX = 280.0f;
    if (hasShield)
    {
        SDL_FRect sBadge = { badgeX, 16.0f, 48.0f, 22.0f };
        SDL_SetRenderDrawColor(renderer, 0x2b, 0x6c, 0xb0, 0xff);
        SDL_RenderFillRect(renderer, &sBadge);
        BitmapFont::drawText(renderer, "SHLD", badgeX + 4.0f, 18.0f, 1.2f, { 255, 255, 255, 255 });
        badgeX += 52.0f;
    }
    if (hasKey)
    {
        SDL_FRect kBadge = { badgeX, 16.0f, 38.0f, 22.0f };
        SDL_SetRenderDrawColor(renderer, 0xd6, 0x9e, 0x2e, 0xff);
        SDL_RenderFillRect(renderer, &kBadge);
        BitmapFont::drawText(renderer, "KEY", badgeX + 4.0f, 18.0f, 1.2f, { 255, 255, 255, 255 });
        badgeX += 42.0f;
    }

    m_btnSound.setPosition(404.0f, 12.0f);
    m_btnSound.setSize(36.0f, 30.0f);
    m_btnSound.render(renderer, soundOn ? SDL_Color{ 45, 55, 72, 255 } : SDL_Color{ 110, 45, 45, 255 });

    // Row 2 (y = 52): Control mode toggle button + Score + Time bar
    std::string ctrlLabel = (controlMode == 0) ? "BOTH" : (controlMode == 1 ? "SWIPE" : "DPAD");
    m_btnControls.setPosition(10.0f, 48.0f);
    m_btnControls.setSize(75.0f, 26.0f);
    m_btnControls.setLabel(ctrlLabel);
    m_btnControls.render(renderer, { 35, 42, 58, 255 }, { 160, 174, 192, 255 });

    ss.str("");
    ss << "PTS: " << score;
    BitmapFont::drawText(renderer, ss.str(), 95.0f, 54.0f, 1.5f, { 104, 219, 120, 255 }); // Green score

    // Time text & progress bar
    if (timeLeft < 0.0f) timeLeft = 0.0f;
    char timeBuffer[16];
    if (freezeTime > 0.0f)
    {
        snprintf(timeBuffer, sizeof(timeBuffer), "FRZ:%.0fs", freezeTime);
    }
    else
    {
        snprintf(timeBuffer, sizeof(timeBuffer), "%.1fs", timeLeft);
    }

    bool isLowTime = (timeLeft < 5.0f && freezeTime <= 0.0f);
    SDL_Color timeColor = (freezeTime > 0.0f) ? SDL_Color{ 100, 220, 255, 255 } : 
                         (isLowTime ? SDL_Color{ 245, 101, 101, 255 } : SDL_Color{ 246, 224, 94, 255 });
    BitmapFont::drawText(renderer, timeBuffer, 222.0f, 54.0f, 1.4f, timeColor);

    // Time Progress Bar
    float barX = 295.0f;
    float barY = 56.0f;
    float barW = 85.0f;
    float barH = 10.0f;

    SDL_FRect bgBar = { barX, barY, barW, barH };
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderFillRect(renderer, &bgBar);

    float ratio = (timeLimit > 0.0f) ? (timeLeft / timeLimit) : 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    if (ratio < 0.0f) ratio = 0.0f;

    SDL_FRect fillBar = { barX, barY, barW * ratio, barH };
    if (freezeTime > 0.0f)
    {
        SDL_SetRenderDrawColor(renderer, 99, 179, 237, 255); // Cyan freeze bar
    }
    else if (ratio > 0.3f)
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
        float tw = BitmapFont::getTextWidth(revText, 1.5f);
        float tx = (Constants::SCREEN_WIDTH - tw) / 2.0f;
        BitmapFont::drawText(renderer, revText, tx, 92.0f, 1.5f, { 255, 255, 255, 255 });
    }
    else if (!debuffMsg.empty())
    {
        SDL_FRect debuffBar = { 15.0f, 88.0f, static_cast<float>(Constants::SCREEN_WIDTH - 30), 24.0f };
        SDL_SetRenderDrawColor(renderer, 0xc5, 0x30, 0x30, 0xdf); // Alert Red
        SDL_RenderFillRect(renderer, &debuffBar);

        float tw = BitmapFont::getTextWidth(debuffMsg, 1.5f);
        float tx = (Constants::SCREEN_WIDTH - tw) / 2.0f;
        BitmapFont::drawText(renderer, debuffMsg, tx, 92.0f, 1.5f, { 255, 255, 255, 255 });
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
    float scale = 3.2f;
    float titleWidth = BitmapFont::getTextWidth(title, scale);
    float titleX = (Constants::SCREEN_WIDTH - titleWidth) / 2.0f;

    // Drop shadow
    BitmapFont::drawText(renderer, title, titleX + 3.0f, 133.0f, scale, { 0, 0, 0, 180 });
    // Title Gold
    BitmapFont::drawText(renderer, title, titleX, 130.0f, scale, { 246, 224, 94, 255 });

    // Subtitle
    std::string sub = "TACTICAL ESCAPE";
    float subScale = 1.6f;
    float subWidth = BitmapFont::getTextWidth(sub, subScale);
    float subX = (Constants::SCREEN_WIDTH - subWidth) / 2.0f;
    BitmapFont::drawText(renderer, sub, subX, 175.0f, subScale, { 160, 174, 192, 255 });

    // Stats Card
    SDL_FRect statCard = { 45.0f, 215.0f, 360.0f, 115.0f };
    SDL_SetRenderDrawColor(renderer, 0x16, 0x1a, 0x24, 0xff);
    SDL_RenderFillRect(renderer, &statCard);
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderRect(renderer, &statCard);

    std::stringstream ss;
    ss << "UNLOCKED LEVEL: " << highestLevel;
    float sw1 = BitmapFont::getTextWidth(ss.str(), 1.7f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw1) / 2.0f, 240.0f, 1.7f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "BEST SCORE: " << highScore;
    float sw2 = BitmapFont::getTextWidth(ss.str(), 1.9f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw2) / 2.0f, 280.0f, 1.9f, { 104, 219, 120, 255 });

    // Menu Buttons
    m_btnPlay.setPosition(85.0f, 365.0f);
    m_btnPlay.setSize(280.0f, 50.0f);
    m_btnPlay.render(renderer, { 72, 187, 120, 255 }); // Bright Green button

    m_btnInfo.setPosition(85.0f, 425.0f);
    m_btnInfo.setSize(280.0f, 45.0f);
    m_btnInfo.setLabel("HOW TO PLAY & INFO");
    m_btnInfo.render(renderer, { 43, 108, 176, 255 }); // Blue info button

    std::string ctrlText = (controlMode == 0) ? "CONTROLS: BOTH" : (controlMode == 1 ? "CONTROLS: SWIPE" : "CONTROLS: DPAD");
    m_btnControls.setPosition(85.0f, 480.0f);
    m_btnControls.setSize(280.0f, 45.0f);
    m_btnControls.setLabel(ctrlText);
    m_btnControls.render(renderer, { 45, 55, 72, 255 }, { 255, 255, 255, 255 });

    m_btnExit.setPosition(85.0f, 535.0f);
    m_btnExit.setSize(280.0f, 45.0f);
    m_btnExit.setLabel("EXIT GAME");
    m_btnExit.render(renderer, { 150, 45, 45, 255 }, { 255, 255, 255, 255 }); // Crimson
}

void HUD::renderPaused(SDL_Renderer* renderer)
{
    // Dark transparent overlay
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xd8); // Dark overlay
    SDL_RenderFillRect(renderer, &overlay);

    std::string pTitle = "GAME PAUSED";
    float pw = BitmapFont::getTextWidth(pTitle, 2.6f);
    BitmapFont::drawText(renderer, pTitle, (Constants::SCREEN_WIDTH - pw) / 2.0f, 180.0f, 2.6f, { 255, 255, 255, 255 });

    m_btnResume.setPosition(85.0f, 240.0f);
    m_btnResume.setSize(280.0f, 48.0f);
    m_btnResume.setLabel("RESUME");
    m_btnResume.render(renderer, { 72, 187, 120, 255 });

    m_btnGiveUp.setPosition(85.0f, 300.0f);
    m_btnGiveUp.setSize(280.0f, 48.0f);
    m_btnGiveUp.setLabel("GIVE UP LEVEL");
    m_btnGiveUp.render(renderer, { 180, 50, 50, 255 });

    m_btnRestartRun.setPosition(85.0f, 360.0f);
    m_btnRestartRun.setSize(280.0f, 48.0f);
    m_btnRestartRun.setLabel("RESTART FROM LEVEL 1");
    m_btnRestartRun.render(renderer, { 180, 100, 30, 255 });

    m_btnMenu.setPosition(85.0f, 420.0f);
    m_btnMenu.setSize(280.0f, 48.0f);
    m_btnMenu.setLabel("MAIN MENU");
    m_btnMenu.render(renderer, { 45, 55, 72, 255 });

    m_btnExit.setPosition(85.0f, 480.0f);
    m_btnExit.setSize(280.0f, 48.0f);
    m_btnExit.setLabel("EXIT GAME");
    m_btnExit.render(renderer, { 120, 35, 35, 255 }); // Dark Red Exit
}

void HUD::renderGameOver(SDL_Renderer* renderer, int level, int score, int highScore, bool timedOut)
{
    // Dark transparent overlay
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x1d, 0x13, 0x13, 0xdf); // Dark Red-tinted overlay
    SDL_RenderFillRect(renderer, &overlay);

    if (timedOut)
    {
        std::string title = "TIME'S UP!";
        float tw = BitmapFont::getTextWidth(title, 3.2f);
        float tx = (Constants::SCREEN_WIDTH - tw) / 2.0f;
        BitmapFont::drawText(renderer, title, tx, 180.0f, 3.2f, { 245, 101, 101, 255 }); // Red

        std::stringstream lvlSS;
        lvlSS << "OUT OF TIME ON LEVEL " << level;
        float subTw = BitmapFont::getTextWidth(lvlSS.str(), 1.8f);
        float subTx = (Constants::SCREEN_WIDTH - subTw) / 2.0f;
        BitmapFont::drawText(renderer, lvlSS.str(), subTx, 240.0f, 1.8f, { 246, 224, 94, 255 }); // Gold
    }
    else
    {
        std::string title = "GAME OVER";
        float tw = BitmapFont::getTextWidth(title, 3.2f);
        float tx = (Constants::SCREEN_WIDTH - tw) / 2.0f;
        BitmapFont::drawText(renderer, title, tx, 180.0f, 3.2f, { 245, 101, 101, 255 }); // Red

        std::stringstream lvlSS;
        lvlSS << "FAILED LEVEL " << level;
        float textWidth = BitmapFont::getTextWidth(lvlSS.str(), 1.8f);
        float ltx = (Constants::SCREEN_WIDTH - textWidth) / 2.0f;
        BitmapFont::drawText(renderer, lvlSS.str(), ltx, 240.0f, 1.8f, { 255, 255, 255, 255 });
    }

    std::stringstream ss;
    ss << "Score: " << score;
    float scW = BitmapFont::getTextWidth(ss.str(), 2.0f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - scW) / 2.0f, 300.0f, 2.0f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "Best:  " << highScore;
    float bestW = BitmapFont::getTextWidth(ss.str(), 2.0f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - bestW) / 2.0f, 340.0f, 2.0f, { 246, 224, 94, 255 });

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

    std::string vTitle = "VICTORY!";
    float vw = BitmapFont::getTextWidth(vTitle, 3.2f);
    BitmapFont::drawText(renderer, vTitle, (Constants::SCREEN_WIDTH - vw) / 2.0f, 120.0f, 3.2f, { 72, 187, 120, 255 }); // Green

    std::stringstream lvlSS;
    lvlSS << "LEVEL " << level << " COMPLETE";
    float textWidth = BitmapFont::getTextWidth(lvlSS.str(), 1.8f);
    float tx = (Constants::SCREEN_WIDTH - textWidth) / 2.0f;
    BitmapFont::drawText(renderer, lvlSS.str(), tx, 175.0f, 1.8f, { 255, 255, 255, 255 });

    // Stars Rating
    // We draw stars using '*' character from our custom bitmap font
    std::string starsStr = "";
    for (int i = 0; i < 3; ++i)
    {
        starsStr += (i < stars) ? " * " : " - ";
    }
    float stW = BitmapFont::getTextWidth(starsStr, 2.5f);
    BitmapFont::drawText(renderer, starsStr, (Constants::SCREEN_WIDTH - stW) / 2.0f, 230.0f, 2.5f, { 246, 224, 94, 255 }); // Gold stars

    // Stats
    std::stringstream ss;
    ss << "Moves: " << moves;
    float mw = BitmapFont::getTextWidth(ss.str(), 1.8f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - mw) / 2.0f, 290.0f, 1.8f, { 255, 255, 255, 255 });

    ss.str("");
    ss << std::fixed << std::setprecision(1) << "Time:  " << time << "s";
    float tw = BitmapFont::getTextWidth(ss.str(), 1.8f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - tw) / 2.0f, 325.0f, 1.8f, { 255, 255, 255, 255 });

    ss.str("");
    ss << "Score: " << score;
    float scW = BitmapFont::getTextWidth(ss.str(), 2.0f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - scW) / 2.0f, 360.0f, 2.0f, { 104, 219, 120, 255 });

    ss.str("");
    ss << "Best:  " << highScore;
    float bW = BitmapFont::getTextWidth(ss.str(), 1.8f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - bW) / 2.0f, 395.0f, 1.8f, { 246, 224, 94, 255 });

    m_btnNext.render(renderer, { 72, 187, 120, 255 }); // Green
    m_btnRestart.setPosition(125.0f, 510.0f);
    m_btnRestart.render(renderer, { 45, 55, 72, 255 }); // Grey-blue
    m_btnMenu.setPosition(125.0f, 580.0f);
    m_btnMenu.render(renderer, { 110, 45, 45, 255 }); // Red
}

void HUD::renderInfo(SDL_Renderer* renderer, int currentTab)
{
    // Full screen background
    SDL_FRect bg = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xff);
    SDL_RenderFillRect(renderer, &bg);

    // Header Title
    std::string title = "GAME GUIDE & INFO";
    float tw = BitmapFont::getTextWidth(title, 2.2f);
    BitmapFont::drawText(renderer, title, (Constants::SCREEN_WIDTH - tw) / 2.0f, 20.0f, 2.2f, { 246, 224, 94, 255 });

    // Tab Buttons at y = 58
    m_btnTabPlay.setPosition(15.0f, 58.0f);
    m_btnTabPlay.setSize(135.0f, 36.0f);
    m_btnTabPlay.render(renderer, (currentTab == 0) ? SDL_Color{ 56, 178, 172, 255 } : SDL_Color{ 35, 42, 58, 255 },
                                  (currentTab == 0) ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 160, 174, 192, 255 });

    m_btnTabTiles.setPosition(157.0f, 58.0f);
    m_btnTabTiles.setSize(135.0f, 36.0f);
    m_btnTabTiles.render(renderer, (currentTab == 1) ? SDL_Color{ 56, 178, 172, 255 } : SDL_Color{ 35, 42, 58, 255 },
                                   (currentTab == 1) ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 160, 174, 192, 255 });

    m_btnTabDev.setPosition(299.0f, 58.0f);
    m_btnTabDev.setSize(135.0f, 36.0f);
    m_btnTabDev.render(renderer, (currentTab == 2) ? SDL_Color{ 56, 178, 172, 255 } : SDL_Color{ 35, 42, 58, 255 },
                                 (currentTab == 2) ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 160, 174, 192, 255 });

    // Content Card Panel
    SDL_FRect card = { 15.0f, 104.0f, static_cast<float>(Constants::SCREEN_WIDTH - 30), 618.0f };
    SDL_SetRenderDrawColor(renderer, 0x18, 0x1d, 0x2b, 0xff);
    SDL_RenderFillRect(renderer, &card);
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderRect(renderer, &card);

    if (currentTab == 0) // HOW TO PLAY
    {
        float cy = 118.0f;
        BitmapFont::drawText(renderer, "MISSION OBJECTIVE", 28.0f, cy, 1.7f, { 72, 187, 120, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "Navigate golden hero to green EXIT", 28.0f, cy, 1.3f, { 220, 230, 242, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "tile before countdown reaches zero!", 28.0f, cy, 1.3f, { 220, 230, 242, 255 });

        cy += 28.0f;
        BitmapFont::drawText(renderer, "CONTROLS & MOVEMENT", 28.0f, cy, 1.7f, { 246, 224, 94, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "- SWIPE 4 directions anywhere on screen.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Or TAP the on-screen tactile D-Pad.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Toggle SWIPE, DPAD, or BOTH anytime.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });

        cy += 28.0f;
        BitmapFont::drawText(renderer, "TIMER & SUDDEN DEATH", 28.0f, cy, 1.7f, { 245, 101, 101, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "- Each level has a strict countdown.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Under 5s: BGM accelerates urgently!", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Out of time = INSTANT GAME OVER!", 28.0f, cy, 1.25f, { 245, 101, 101, 255 });

        cy += 28.0f;
        BitmapFont::drawText(renderer, "TACTICAL BUTTONS", 28.0f, cy, 1.7f, { 99, 179, 237, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "- [GIVE]: Concede & retry current level.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- [LVL 1]: Restart run from Level 1.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- [II]: Pause game and access menu.", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });

        cy += 28.0f;
        BitmapFont::drawText(renderer, "SCORING & 3-STAR SYSTEM", 28.0f, cy, 1.7f, { 246, 224, 94, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "- Complete levels in fewer moves and faster", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "  times to unlock 3 STARS and high score!", 28.0f, cy, 1.25f, { 200, 210, 225, 255 });
    }
    else if (currentTab == 1) // ALL TILES & ITEMS
    {
        struct TileInfo {
            std::string name;
            std::string desc;
            SDL_Color col;
        };

        TileInfo items[] = {
            { "EXIT",     "Goal tile! Step here to finish level", { 72, 187, 120, 255 } },
            { "DANGER",   "Fatal spikes & traps on contact!",     { 245, 101, 101, 255 } },
            { "ICE",      "Slick ice. Slide until obstacle",      { 118, 228, 247, 255 } },
            { "CRUMB",    "Fragile stone. Falls into pit!",       { 214, 158, 46, 255 } },
            { "PIT",      "Bottomless void. Instant death!",      { 120, 120, 140, 255 } },
            { "PORTAL",   "Cosmic rift linking two portals",      { 183, 148, 244, 255 } },
            { "KEY/GATE", "Golden key unlatches locked gate",     { 250, 204, 21, 255 } },
            { "BOMB",     "Detonates 3x3 blast clearing walls",   { 140, 150, 170, 255 } },
            { "SHIELD",   "Protective crest absorbs 1 death",     { 49, 130, 206, 255 } },
            { "FREEZE",   "Hourglass pauses timer for 8.0s",      { 0, 181, 216, 255 } },
            { "COIN",     "Shiny bonus treasure grants +500 PTS", { 250, 204, 21, 255 } },
            { "CURSE",    "Purple debuff reverses hero moves",    { 213, 63, 140, 255 } },
            { "DEFUSE",   "Teal cross cures curse & traps",       { 56, 178, 172, 255 } }
        };

        float startY = 112.0f;
        float rowH = 46.0f;

        for (int i = 0; i < 13; ++i)
        {
            float ry = startY + i * rowH;

            // Mini visual icon
            SDL_FRect iconRect = { 26.0f, ry + 2.0f, 22.0f, 22.0f };
            SDL_SetRenderDrawColor(renderer, items[i].col.r, items[i].col.g, items[i].col.b, 255);
            SDL_RenderFillRect(renderer, &iconRect);

            // Name
            BitmapFont::drawText(renderer, items[i].name, 56.0f, ry + 2.0f, 1.5f, items[i].col);

            // Desc
            BitmapFont::drawText(renderer, items[i].desc, 56.0f, ry + 22.0f, 1.25f, { 190, 205, 225, 255 });
        }
    }
    else if (currentTab == 2) // DEVELOPER INFO
    {
        float cy = 120.0f;
        BitmapFont::drawText(renderer, "ONE MORE MOVE", 28.0f, cy, 2.4f, { 246, 224, 94, 255 });
        cy += 28.0f;
        BitmapFont::drawText(renderer, "TACTICAL ESCAPE - RETRO EDITION", 28.0f, cy, 1.35f, { 160, 174, 192, 255 });

        cy += 30.0f;
        BitmapFont::drawText(renderer, "CREATOR & LEAD DEVELOPER", 28.0f, cy, 1.8f, { 72, 187, 120, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "Prasad Kankhar", 28.0f, cy, 1.6f, { 255, 255, 255, 255 });
        cy += 20.0f;
        BitmapFont::drawText(renderer, "Lead Game Designer & Programmer", 28.0f, cy, 1.3f, { 180, 195, 215, 255 });

        cy += 30.0f;
        BitmapFont::drawText(renderer, "ENGINE & TECH ARCHITECTURE", 28.0f, cy, 1.8f, { 99, 179, 237, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "- Language: Modern ISO C++20", 28.0f, cy, 1.3f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Graphics: SDL3 Vector & Geometry", 28.0f, cy, 1.3f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Font: Custom Code-Synthesized CP437", 28.0f, cy, 1.3f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Audio: 100% Procedural 8-Bit Chiptune", 28.0f, cy, 1.3f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- APK Size: Ultra-light (<5 MB)", 28.0f, cy, 1.3f, { 200, 210, 225, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "- Platform: Android Native & Desktop", 28.0f, cy, 1.3f, { 200, 210, 225, 255 });

        cy += 30.0f;
        BitmapFont::drawText(renderer, "DESIGN PHILOSOPHY", 28.0f, cy, 1.8f, { 246, 224, 94, 255 });
        cy += 24.0f;
        BitmapFont::drawText(renderer, "Built to deliver the pure, intense thrill", 28.0f, cy, 1.3f, { 220, 230, 245, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "of retro tactical gaming where every single", 28.0f, cy, 1.3f, { 220, 230, 245, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "move counts. Think fast, plan, and escape!", 28.0f, cy, 1.3f, { 220, 230, 245, 255 });
    }

    // Back to Menu Button
    m_btnBack.setPosition(85.0f, 735.0f);
    m_btnBack.setSize(280.0f, 48.0f);
    m_btnBack.setLabel("BACK TO MENU");
    m_btnBack.render(renderer, { 72, 187, 120, 255 });
}
