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
    cleanupTextures();
}

void HUD::loadTextures(SDL_Renderer* renderer)
{
    if (!m_logoTexture && renderer)
    {
        SDL_Surface* surface = SDL_LoadBMP("assets/logo.bmp");
        if (surface)
        {
            m_logoTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroySurface(surface);
        }
    }
}

void HUD::cleanupTextures()
{
    if (m_logoTexture)
    {
        SDL_DestroyTexture(m_logoTexture);
        m_logoTexture = nullptr;
    }
}

void HUD::drawStar(SDL_Renderer* renderer, float cx, float cy, float radius, SDL_Color color)
{
    SDL_Vertex vertices[11];
    vertices[0].position = { cx, cy };
    vertices[0].color = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    float innerRadius = radius * 0.42f;
    for (int i = 0; i < 10; ++i)
    {
        float angle = i * 3.14159265f / 5.0f - 1.5707963f;
        float r = (i % 2 == 0) ? radius : innerRadius;
        vertices[i + 1].position = { cx + std::cos(angle) * r, cy + std::sin(angle) * r };
        vertices[i + 1].color = vertices[0].color;
    }

    int indices[30];
    for (int i = 0; i < 10; ++i)
    {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = (i == 9) ? 1 : (i + 2);
    }

    SDL_RenderGeometry(renderer, nullptr, vertices, 11, indices, 30);
}

void HUD::drawLogoBadge(SDL_Renderer* renderer, float cx, float cy, float scale)
{
    float r = 24.0f * scale;

    // Outer glow hex outline
    SDL_SetRenderDrawColor(renderer, 56, 178, 172, 180);
    for (int i = 0; i < 6; ++i)
    {
        float a1 = i * 3.14159265f / 3.0f - 1.5707963f;
        float a2 = (i + 1) * 3.14159265f / 3.0f - 1.5707963f;
        SDL_RenderLine(renderer, cx + std::cos(a1) * (r + 4.0f), cy + std::sin(a1) * (r + 4.0f),
                                cx + std::cos(a2) * (r + 4.0f), cy + std::sin(a2) * (r + 4.0f));
    }

    // Inner filled hexagon
    SDL_Vertex hexVerts[7];
    hexVerts[0].position = { cx, cy };
    hexVerts[0].color = { 0.10f, 0.13f, 0.20f, 1.0f };
    for (int i = 0; i < 6; ++i)
    {
        float a = i * 3.14159265f / 3.0f - 1.5707963f;
        hexVerts[i + 1].position = { cx + std::cos(a) * r, cy + std::sin(a) * r };
        hexVerts[i + 1].color = { 0.16f, 0.22f, 0.32f, 1.0f };
    }
    int hexIdx[18];
    for (int i = 0; i < 6; ++i)
    {
        hexIdx[i * 3 + 0] = 0;
        hexIdx[i * 3 + 1] = i + 1;
        hexIdx[i * 3 + 2] = (i == 5) ? 1 : (i + 2);
    }
    SDL_RenderGeometry(renderer, nullptr, hexVerts, 7, hexIdx, 18);

    // Inner gold star core
    drawStar(renderer, cx, cy, 11.0f * scale, { 250, 204, 21, 255 });
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
    m_btnPlay = Button(85.0f, 310.0f, 280.0f, 46.0f, "PLAY CAMPAIGN");
    m_btnLevelSelect = Button(85.0f, 365.0f, 280.0f, 46.0f, "SELECT LEVEL");
    m_btnInfo = Button(85.0f, 420.0f, 280.0f, 46.0f, "HOW TO PLAY & INFO");
    m_btnSettings = Button(85.0f, 475.0f, 280.0f, 46.0f, "SETTINGS");
    m_btnExit = Button(85.0f, 530.0f, 280.0f, 46.0f, "EXIT GAME");
    m_btnEndless = Button(85.0f, 585.0f, 280.0f, 46.0f, "ENDLESS EXPEDITION");

    m_btnResume = Button(85.0f, 240.0f, 280.0f, 48.0f, "RESUME");
    m_btnNext = Button(115.0f, 440.0f, 220.0f, 55.0f, "NEXT LEVEL");
    m_btnRestart = Button(85.0f, 400.0f, 280.0f, 46.0f, "TRY AGAIN");
    m_btnShowPath = Button(85.0f, 455.0f, 280.0f, 46.0f, "SHOW WINNING PATH");
    m_btnMenu = Button(85.0f, 510.0f, 280.0f, 46.0f, "MAIN MENU");

    // Settings screen buttons
    m_btnControls = Button(65.0f, 180.0f, 320.0f, 48.0f, "CONTROLS: BOTH");
    m_btnHaptics = Button(65.0f, 245.0f, 320.0f, 48.0f, "HAPTICS: ON");
    m_btnResetData = Button(65.0f, 375.0f, 320.0f, 48.0f, "RESET ALL PROGRESS");

    // Level Select buttons: 4 cols x 6 rows (levels 1 to 24)
    float gridLeft = 25.0f;
    float gridTop = 110.0f;
    float lvlBtnW = 90.0f;
    float lvlBtnH = 68.0f;
    float gapX = 13.0f;
    float gapY = 16.0f;

    for (int r = 0; r < 6; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            int lvl = r * 4 + c + 1;
            float bx = gridLeft + c * (lvlBtnW + gapX);
            float by = gridTop + r * (lvlBtnH + gapY);
            m_btnLevels[lvl - 1] = Button(bx, by, lvlBtnW, lvlBtnH, std::to_string(lvl));
        }
    }

    // Info screen tab buttons
    m_btnTabPlay = Button(15.0f, 65.0f, 135.0f, 36.0f, "HOW TO PLAY");
    m_btnTabTiles = Button(157.0f, 65.0f, 135.0f, 36.0f, "ALL TILES");
    m_btnTabDev = Button(299.0f, 65.0f, 135.0f, 36.0f, "DEVELOPER");
    m_btnBack = Button(85.0f, 735.0f, 280.0f, 48.0f, "BACK TO MENU");
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

void HUD::renderMainMenu(SDL_Renderer* renderer, int highScore, int highestLevel, int controlMode, int totalStars, bool campaignCompleted)
{
    if (m_logoTexture)
    {
        // Render crisp logo image (1024x512 aspect ratio = 2:1) centered at top
        SDL_FRect logoRect = { 100.0f, 10.0f, 250.0f, 125.0f };
        SDL_RenderTexture(renderer, m_logoTexture, nullptr, &logoRect);
    }
    else
    {
        // Draw Emblem Logo Badge at top of screen
        drawLogoBadge(renderer, Constants::SCREEN_WIDTH / 2.0f, 36.0f, 0.9f);

        // Draw Title: ONE MORE MOVE with shadow
        std::string title = "ONE MORE MOVE";
        float scale = 3.0f;
        float titleWidth = BitmapFont::getTextWidth(title, scale);
        float titleX = (Constants::SCREEN_WIDTH - titleWidth) / 2.0f;

        // Drop shadow
        BitmapFont::drawText(renderer, title, titleX + 2.0f, 70.0f, scale, { 0, 0, 0, 180 });
        // Title Gold
        BitmapFont::drawText(renderer, title, titleX, 68.0f, scale, { 246, 224, 94, 255 });

        // Subtitle
        std::string sub = "TACTICAL ESCAPE";
        float subScale = 1.5f;
        float subWidth = BitmapFont::getTextWidth(sub, subScale);
        float subX = (Constants::SCREEN_WIDTH - subWidth) / 2.0f;
        BitmapFont::drawText(renderer, sub, subX, 108.0f, subScale, { 160, 174, 192, 255 });
    }

    // Stats Card
    SDL_FRect statCard = { 45.0f, 142.0f, 360.0f, 125.0f };
    SDL_SetRenderDrawColor(renderer, 0x16, 0x1a, 0x24, 0xff);
    SDL_RenderFillRect(renderer, &statCard);
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderRect(renderer, &statCard);

    std::stringstream ss;
    if (campaignCompleted)
    {
        ss << "! CAMPAIGN CONQUERED !";
        float sw1 = BitmapFont::getTextWidth(ss.str(), 1.6f);
        BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw1) / 2.0f, 158.0f, 1.6f, { 72, 187, 120, 255 });
    }
    else
    {
        ss << "CAMPAIGN: " << std::min(highestLevel, 24) << " / 24 UNLOCKED";
        float sw1 = BitmapFont::getTextWidth(ss.str(), 1.5f);
        BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw1) / 2.0f, 158.0f, 1.5f, { 246, 224, 94, 255 });
    }

    ss.str("");
    ss << "STARS COLLECTED: " << totalStars << " / 72";
    float sw2 = BitmapFont::getTextWidth(ss.str(), 1.5f);
    float starLineX = (Constants::SCREEN_WIDTH - sw2) / 2.0f;
    BitmapFont::drawText(renderer, ss.str(), starLineX, 192.0f, 1.5f, { 250, 204, 21, 255 });
    drawStar(renderer, starLineX - 14.0f, 198.0f, 6.5f, { 250, 204, 21, 255 });
    drawStar(renderer, starLineX + sw2 + 14.0f, 198.0f, 6.5f, { 250, 204, 21, 255 });

    ss.str("");
    ss << "BEST SCORE: " << highScore;
    float sw3 = BitmapFont::getTextWidth(ss.str(), 1.6f);
    BitmapFont::drawText(renderer, ss.str(), (Constants::SCREEN_WIDTH - sw3) / 2.0f, 226.0f, 1.6f, { 104, 219, 120, 255 });

    // Menu Buttons
    m_btnPlay.setPosition(85.0f, 288.0f);
    m_btnPlay.setSize(280.0f, 48.0f);
    m_btnPlay.setLabel(highestLevel > 1 ? "RESUME CAMPAIGN" : "PLAY CAMPAIGN");
    m_btnPlay.render(renderer, { 72, 187, 120, 255 }); // Bright Green button

    m_btnLevelSelect.setPosition(85.0f, 346.0f);
    m_btnLevelSelect.setSize(280.0f, 46.0f);
    m_btnLevelSelect.setLabel("SELECT MISSION (1-24)");
    m_btnLevelSelect.render(renderer, { 56, 178, 172, 255 }); // Teal button

    m_btnInfo.setPosition(85.0f, 402.0f);
    m_btnInfo.setSize(280.0f, 46.0f);
    m_btnInfo.setLabel("HOW TO PLAY & GUIDE");
    m_btnInfo.render(renderer, { 43, 108, 176, 255 }); // Blue info button

    m_btnSettings.setPosition(85.0f, 458.0f);
    m_btnSettings.setSize(280.0f, 46.0f);
    m_btnSettings.setLabel("SETTINGS & AUDIO");
    m_btnSettings.render(renderer, { 45, 55, 72, 255 }); // Slate button

    if (campaignCompleted)
    {
        m_btnEndless.setPosition(85.0f, 514.0f);
        m_btnEndless.setSize(280.0f, 46.0f);
        m_btnEndless.setLabel("ENDLESS EXPEDITION");
        m_btnEndless.render(renderer, { 180, 100, 30, 255 }, { 255, 230, 180, 255 }); // Gold Endless

        m_btnExit.setPosition(85.0f, 570.0f);
        m_btnExit.setSize(280.0f, 46.0f);
        m_btnExit.setLabel("EXIT GAME");
        m_btnExit.render(renderer, { 150, 45, 45, 255 }); // Crimson
    }
    else
    {
        m_btnExit.setPosition(85.0f, 514.0f);
        m_btnExit.setSize(280.0f, 46.0f);
        m_btnExit.setLabel("EXIT GAME");
        m_btnExit.render(renderer, { 150, 45, 45, 255 }); // Crimson
    }
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

    m_btnRestart.setPosition(85.0f, 400.0f);
    m_btnRestart.setSize(280.0f, 46.0f);
    m_btnRestart.setLabel("TRY AGAIN");
    m_btnRestart.render(renderer, { 72, 187, 120, 255 }); // Green

    m_btnShowPath.setPosition(85.0f, 455.0f);
    m_btnShowPath.setSize(280.0f, 46.0f);
    m_btnShowPath.setLabel("SHOW WINNING PATH");
    m_btnShowPath.render(renderer, { 200, 145, 30, 255 }, { 255, 255, 255, 255 }); // Gold Challenge button

    m_btnMenu.setPosition(85.0f, 510.0f);
    m_btnMenu.setSize(280.0f, 46.0f);
    m_btnMenu.setLabel("MAIN MENU");
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

    // Stars Rating: 3 large glowing vector stars
    float starCenter = Constants::SCREEN_WIDTH / 2.0f;
    float starY = 228.0f;
    float starGap = 44.0f;
    for (int i = 0; i < 3; ++i)
    {
        float sx = starCenter + (i - 1) * starGap;
        bool earned = (i < stars);
        if (earned)
        {
            drawStar(renderer, sx, starY, 19.0f, { 255, 230, 80, 110 }); // Glow aura
            drawStar(renderer, sx, starY, 15.0f, { 250, 204, 21, 255 }); // Gold core
        }
        else
        {
            drawStar(renderer, sx, starY, 14.0f, { 55, 65, 80, 255 }); // Dimmed unfilled
        }
    }

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
        float maxW = card.w - 28.0f;

        BitmapFont::drawText(renderer, "MISSION OBJECTIVE", 28.0f, cy, 1.6f, { 72, 187, 120, 255 });
        cy += 22.0f;
        cy = BitmapFont::drawTextWrapped(renderer, "Navigate golden hero to green EXIT tile before the countdown reaches zero!", 28.0f, cy, maxW, 1.2f, { 220, 230, 242, 255 }, 16.0f);

        cy += 12.0f;
        BitmapFont::drawText(renderer, "CONTROLS & MOVEMENT", 28.0f, cy, 1.6f, { 246, 224, 94, 255 });
        cy += 22.0f;
        cy = BitmapFont::drawTextWrapped(renderer, "- Swipe in 4 directions anywhere on screen.\n- Or tap the on-screen tactile D-Pad.\n- Toggle control modes anytime in Settings.", 28.0f, cy, maxW, 1.15f, { 200, 210, 225, 255 }, 16.0f);

        cy += 12.0f;
        BitmapFont::drawText(renderer, "TIMER & SUDDEN DEATH", 28.0f, cy, 1.6f, { 245, 101, 101, 255 });
        cy += 22.0f;
        cy = BitmapFont::drawTextWrapped(renderer, "- Each level has a strict countdown timer.\n- Under 5s: BGM accelerates urgently!\n- Out of time = Sudden Death Game Over!", 28.0f, cy, maxW, 1.15f, { 200, 210, 225, 255 }, 16.0f);

        cy += 12.0f;
        BitmapFont::drawText(renderer, "TACTICAL BUTTONS", 28.0f, cy, 1.6f, { 99, 179, 237, 255 });
        cy += 22.0f;
        cy = BitmapFont::drawTextWrapped(renderer, "- [GIVE]: Concede and retry current level.\n- [LVL 1]: Restart run from Level 1.\n- [II]: Pause game and access settings.", 28.0f, cy, maxW, 1.15f, { 200, 210, 225, 255 }, 16.0f);

        cy += 12.0f;
        BitmapFont::drawText(renderer, "SCORING & 3-STAR RATING", 28.0f, cy, 1.6f, { 246, 224, 94, 255 });
        cy += 22.0f;
        cy = BitmapFont::drawTextWrapped(renderer, "Clear levels in fewer moves and faster times to unlock 3 STARS and set record scores!", 28.0f, cy, maxW, 1.15f, { 200, 210, 225, 255 }, 16.0f);
    }
    else if (currentTab == 1) // ALL TILES & ITEMS
    {
        struct TileInfo {
            std::string name;
            std::string desc;
            SDL_Color col;
        };

        TileInfo items[] = {
            { "EXIT",     "Goal tile! Step here to clear level",  { 72, 187, 120, 255 } },
            { "DANGER",   "Fatal traps & spikes on contact!",     { 245, 101, 101, 255 } },
            { "ICE",      "Slick ice. Slide until obstacle",      { 118, 228, 247, 255 } },
            { "CRUMB",    "Fragile stone. Collapses into pit!",   { 214, 158, 46, 255 } },
            { "PIT",      "Bottomless void. Instant death!",      { 120, 120, 140, 255 } },
            { "PORTAL",   "Cosmic rift linking paired portals",   { 183, 148, 244, 255 } },
            { "KEY/GATE", "Golden key unlatches locked gate",     { 250, 204, 21, 255 } },
            { "BOMB",     "Detonates 3x3 blast clearing walls",   { 140, 150, 170, 255 } },
            { "SHIELD",   "Energy crest absorbs 1 death hit",     { 49, 130, 206, 255 } },
            { "FREEZE",   "Hourglass freezes time for 8.0s",      { 0, 181, 216, 255 } },
            { "COIN",     "Bonus treasure grants +500 PTS",       { 250, 204, 21, 255 } },
            { "CURSE",    "Purple debuff reverses controls",      { 213, 63, 140, 255 } },
            { "DEFUSE",   "Teal cross disarms nearby traps",      { 56, 178, 172, 255 } }
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
            BitmapFont::drawText(renderer, items[i].name, 56.0f, ry + 2.0f, 1.4f, items[i].col);

            // Desc
            BitmapFont::drawText(renderer, items[i].desc, 56.0f, ry + 20.0f, 1.15f, { 190, 205, 225, 255 });
        }
    }
    else if (currentTab == 2) // DEVELOPER INFO
    {
        float cy = 118.0f;
        float maxW = card.w - 28.0f;

        BitmapFont::drawText(renderer, "ONE MORE MOVE", 28.0f, cy, 2.2f, { 246, 224, 94, 255 });
        cy += 26.0f;
        BitmapFont::drawText(renderer, "TACTICAL ESCAPE - RETRO EDITION", 28.0f, cy, 1.25f, { 160, 174, 192, 255 });

        cy += 28.0f;
        BitmapFont::drawText(renderer, "CREATOR & LEAD DEVELOPER", 28.0f, cy, 1.6f, { 72, 187, 120, 255 });
        cy += 22.0f;
        BitmapFont::drawText(renderer, "Prasad Kankhar", 28.0f, cy, 1.5f, { 255, 255, 255, 255 });
        cy += 18.0f;
        BitmapFont::drawText(renderer, "Lead Game Designer & Programmer", 28.0f, cy, 1.2f, { 180, 195, 215, 255 });

        cy += 26.0f;
        BitmapFont::drawText(renderer, "ENGINE & TECH ARCHITECTURE", 28.0f, cy, 1.6f, { 99, 179, 237, 255 });
        cy += 22.0f;
        BitmapFont::drawText(renderer, "- Language: Modern ISO C++20", 28.0f, cy, 1.2f, { 200, 210, 225, 255 });
        cy += 17.0f;
        BitmapFont::drawText(renderer, "- Graphics: SDL3 Vector & Geometry", 28.0f, cy, 1.2f, { 200, 210, 225, 255 });
        cy += 17.0f;
        BitmapFont::drawText(renderer, "- Font: Custom Code-Synthesized CP437", 28.0f, cy, 1.2f, { 200, 210, 225, 255 });
        cy += 17.0f;
        BitmapFont::drawText(renderer, "- Audio: 100% Procedural 8-Bit Chiptune", 28.0f, cy, 1.2f, { 200, 210, 225, 255 });
        cy += 17.0f;
        BitmapFont::drawText(renderer, "- APK Size: Ultra-light (<5 MB)", 28.0f, cy, 1.2f, { 200, 210, 225, 255 });
        cy += 17.0f;
        BitmapFont::drawText(renderer, "- Platform: Android Native & Desktop", 28.0f, cy, 1.2f, { 200, 210, 225, 255 });

        cy += 26.0f;
        BitmapFont::drawText(renderer, "DESIGN PHILOSOPHY", 28.0f, cy, 1.6f, { 246, 224, 94, 255 });
        cy += 22.0f;
        std::string philosophy = "Built to deliver the pure, intense thrill of retro tactical gaming where every single move counts. Think fast, plan, and escape!";
        cy = BitmapFont::drawTextWrapped(renderer, philosophy, 28.0f, cy, maxW, 1.15f, { 220, 230, 245, 255 }, 18.0f);
    }

    // Back to Menu Button
    m_btnBack.setPosition(85.0f, 735.0f);
    m_btnBack.setSize(280.0f, 48.0f);
    m_btnBack.setLabel("BACK TO MENU");
    m_btnBack.render(renderer, { 72, 187, 120, 255 });
}

void HUD::renderPathPreview(SDL_Renderer* renderer, int pathMoves)
{
    // Top banner overlay with challenge info
    SDL_FRect topBar = { 15.0f, 15.0f, static_cast<float>(Constants::SCREEN_WIDTH - 30), 65.0f };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xee);
    SDL_RenderFillRect(renderer, &topBar);
    SDL_SetRenderDrawColor(renderer, 0x2d, 0x35, 0x48, 0xff);
    SDL_RenderRect(renderer, &topBar);

    std::string routeText = "OPTIMAL ROUTE: " + std::to_string(pathMoves) + " MOVES";
    float tw1 = BitmapFont::getTextWidth(routeText, 1.8f);
    BitmapFont::drawText(renderer, routeText, (Constants::SCREEN_WIDTH - tw1) / 2.0f, 24.0f, 1.8f, { 246, 224, 94, 255 });

    std::string subText = "STUDY THE PATH & MASTER THE LEVEL";
    float tw2 = BitmapFont::getTextWidth(subText, 1.25f);
    BitmapFont::drawText(renderer, subText, (Constants::SCREEN_WIDTH - tw2) / 2.0f, 52.0f, 1.25f, { 72, 187, 120, 255 });

    // Bottom action button: RETRY CHALLENGE
    m_btnRestart.setPosition(85.0f, 735.0f);
    m_btnRestart.setSize(280.0f, 48.0f);
    m_btnRestart.setLabel("RETRY CHALLENGE");
    m_btnRestart.render(renderer, { 72, 187, 120, 255 });
}

void HUD::renderLevelSelect(SDL_Renderer* renderer, int highestLevel, const int levelStars[25])
{
    // Full screen background
    SDL_FRect bg = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xff);
    SDL_RenderFillRect(renderer, &bg);

    // Title
    std::string title = "CAMPAIGN MISSIONS";
    float tw = BitmapFont::getTextWidth(title, 2.3f);
    BitmapFont::drawText(renderer, title, (Constants::SCREEN_WIDTH - tw) / 2.0f, 25.0f, 2.3f, { 246, 224, 94, 255 });

    std::string sub = "CHOOSE A MISSION TO CONQUER (1-24)";
    float subW = BitmapFont::getTextWidth(sub, 1.25f);
    BitmapFont::drawText(renderer, sub, (Constants::SCREEN_WIDTH - subW) / 2.0f, 60.0f, 1.25f, { 160, 174, 192, 255 });

    // 8 Biome color accents
    SDL_Color biomeColors[8] = {
        { 60, 70, 90, 255 },   // 1-3: Midnight Dungeon
        { 38, 80, 48, 255 },   // 4-6: Mossy Ruins
        { 25, 75, 100, 255 },  // 7-9: Ocean Abyss
        { 130, 50, 25, 255 },  // 10-12: Volcanic Forge
        { 45, 90, 120, 255 },  // 13-15: Glacial Cavern
        { 90, 40, 110, 255 },  // 16-18: Neon Cyberpunk
        { 130, 100, 35, 255 }, // 19-21: Pharaoh Tomb
        { 75, 35, 120, 255 }   // 22-24: Cosmic Void
    };

    for (int lvl = 1; lvl <= 24; ++lvl)
    {
        bool unlocked = (lvl <= highestLevel);
        int biomeIdx = (lvl - 1) / 3;
        if (biomeIdx < 0) biomeIdx = 0;
        if (biomeIdx > 7) biomeIdx = 7;

        SDL_Color bgCol = unlocked ? biomeColors[biomeIdx] : SDL_Color{ 25, 30, 40, 255 };
        SDL_Color txtCol = unlocked ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 90, 100, 115, 255 };

        std::string lvlLabel = unlocked ? std::to_string(lvl) : "LOCK";
        m_btnLevels[lvl - 1].setLabel(lvlLabel);
        m_btnLevels[lvl - 1].render(renderer, bgCol, txtCol);

        // Render 3 vector stars below level number if unlocked
        if (unlocked)
        {
            int s = levelStars[lvl];
            float starBaseY = m_btnLevels[lvl - 1].getY() + m_btnLevels[lvl - 1].getH() - 11.0f;
            float btnCenterX = m_btnLevels[lvl - 1].getX() + m_btnLevels[lvl - 1].getW() / 2.0f;
            float starSpacing = 16.0f;
            for (int i = 0; i < 3; ++i)
            {
                float sx = btnCenterX + (i - 1) * starSpacing;
                SDL_Color starCol = (i < s) ? SDL_Color{ 250, 204, 21, 255 } : SDL_Color{ 60, 70, 85, 255 };
                drawStar(renderer, sx, starBaseY, 4.2f, starCol);
            }
        }
    }

    // Back to Menu button
    m_btnBack.setPosition(85.0f, 735.0f);
    m_btnBack.setSize(280.0f, 48.0f);
    m_btnBack.setLabel("BACK TO MENU");
    m_btnBack.render(renderer, { 72, 187, 120, 255 });
}

void HUD::renderSettings(SDL_Renderer* renderer, bool soundOn, int controlMode, bool hapticsOn, bool confirmReset)
{
    // Full screen background
    SDL_FRect bg = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, 0xff);
    SDL_RenderFillRect(renderer, &bg);

    std::string title = "GAME SETTINGS";
    float tw = BitmapFont::getTextWidth(title, 2.4f);
    BitmapFont::drawText(renderer, title, (Constants::SCREEN_WIDTH - tw) / 2.0f, 45.0f, 2.4f, { 246, 224, 94, 255 });

    std::string sub = "CUSTOMIZE YOUR TACTICAL CONTROLS";
    float subW = BitmapFont::getTextWidth(sub, 1.25f);
    BitmapFont::drawText(renderer, sub, (Constants::SCREEN_WIDTH - subW) / 2.0f, 85.0f, 1.25f, { 160, 174, 192, 255 });

    // Option 1: Controls
    std::string ctrlLabel = (controlMode == 0) ? "CONTROLS: BOTH (SWIPE+DPAD)" : 
                           (controlMode == 1 ? "CONTROLS: SWIPE ONLY" : "CONTROLS: DPAD ONLY");
    m_btnControls.setPosition(45.0f, 150.0f);
    m_btnControls.setSize(360.0f, 52.0f);
    m_btnControls.setLabel(ctrlLabel);
    m_btnControls.render(renderer, { 45, 55, 72, 255 }, { 255, 255, 255, 255 });

    // Option 2: Sound
    std::string sndLabel = soundOn ? "8-BIT CHIPTUNE AUDIO: ON" : "8-BIT CHIPTUNE AUDIO: OFF";
    m_btnSound.setPosition(45.0f, 220.0f);
    m_btnSound.setSize(360.0f, 52.0f);
    m_btnSound.setLabel(sndLabel);
    m_btnSound.render(renderer, soundOn ? SDL_Color{ 43, 108, 176, 255 } : SDL_Color{ 110, 45, 45, 255 });

    // Option 3: Haptics
    std::string hapLabel = hapticsOn ? "HAPTIC VIBRATION: ON" : "HAPTIC VIBRATION: OFF";
    m_btnHaptics.setPosition(45.0f, 290.0f);
    m_btnHaptics.setSize(360.0f, 52.0f);
    m_btnHaptics.setLabel(hapLabel);
    m_btnHaptics.render(renderer, hapticsOn ? SDL_Color{ 56, 178, 172, 255 } : SDL_Color{ 80, 80, 80, 255 });

    // Option 4: Reset data
    std::string resetLabel = confirmReset ? "! TAP AGAIN TO CONFIRM RESET !" : "RESET ALL GAME PROGRESS";
    m_btnResetData.setPosition(45.0f, 380.0f);
    m_btnResetData.setSize(360.0f, 52.0f);
    m_btnResetData.setLabel(resetLabel);
    m_btnResetData.render(renderer, confirmReset ? SDL_Color{ 200, 40, 40, 255 } : SDL_Color{ 100, 30, 30, 255 },
                                   { 255, 220, 220, 255 });

    // Back button
    m_btnBack.setPosition(85.0f, 735.0f);
    m_btnBack.setSize(280.0f, 48.0f);
    m_btnBack.setLabel("BACK TO MENU");
    m_btnBack.render(renderer, { 72, 187, 120, 255 });
}

void HUD::renderGameWon(SDL_Renderer* renderer, int totalScore, int totalStars)
{
    // Full screen overlay with celestial background
    SDL_FRect bg = { 0.0f, 0.0f, static_cast<float>(Constants::SCREEN_WIDTH), static_cast<float>(Constants::SCREEN_HEIGHT) };
    SDL_SetRenderDrawColor(renderer, 0x0f, 0x11, 0x1a, 0xff);
    SDL_RenderFillRect(renderer, &bg);

    std::string title = "CAMPAIGN CONQUERED!";
    float tw = BitmapFont::getTextWidth(title, 2.2f);
    BitmapFont::drawText(renderer, title, (Constants::SCREEN_WIDTH - tw) / 2.0f, 50.0f, 2.2f, { 246, 224, 94, 255 });

    std::string sub = "ALL 24 BIOME MISSIONS COMPLETED!";
    float subW = BitmapFont::getTextWidth(sub, 1.25f);
    BitmapFont::drawText(renderer, sub, (Constants::SCREEN_WIDTH - subW) / 2.0f, 90.0f, 1.25f, { 72, 187, 120, 255 });

    // Victory Card
    SDL_FRect card = { 35.0f, 130.0f, static_cast<float>(Constants::SCREEN_WIDTH - 70), 320.0f };
    SDL_SetRenderDrawColor(renderer, 0x18, 0x1d, 0x2b, 0xff);
    SDL_RenderFillRect(renderer, &card);
    SDL_SetRenderDrawColor(renderer, 0xf6, 0xe0, 0x5e, 0xff);
    SDL_RenderRect(renderer, &card);

    float cy = 155.0f;
    std::string s1 = "TOTAL STARS: " + std::to_string(totalStars) + " / 72";
    float w1 = BitmapFont::getTextWidth(s1, 1.7f);
    float s1X = (Constants::SCREEN_WIDTH - w1) / 2.0f;
    BitmapFont::drawText(renderer, s1, s1X, cy, 1.7f, { 250, 204, 21, 255 });
    drawStar(renderer, s1X - 15.0f, cy + 8.0f, 7.0f, { 250, 204, 21, 255 });
    drawStar(renderer, s1X + w1 + 15.0f, cy + 8.0f, 7.0f, { 250, 204, 21, 255 });

    cy += 40.0f;
    std::string s2 = "FINAL SCORE: " + std::to_string(totalScore);
    float w2 = BitmapFont::getTextWidth(s2, 1.8f);
    BitmapFont::drawText(renderer, s2, (Constants::SCREEN_WIDTH - w2) / 2.0f, cy, 1.8f, { 104, 219, 120, 255 });

    cy += 45.0f;
    std::string s3 = "! ENDLESS MODE UNLOCKED !";
    float w3 = BitmapFont::getTextWidth(s3, 1.6f);
    BitmapFont::drawText(renderer, s3, (Constants::SCREEN_WIDTH - w3) / 2.0f, cy, 1.6f, { 99, 179, 237, 255 });

    cy += 35.0f;
    std::string s4 = "Survive infinite scaling dungeons!";
    float w4 = BitmapFont::getTextWidth(s4, 1.25f);
    BitmapFont::drawText(renderer, s4, (Constants::SCREEN_WIDTH - w4) / 2.0f, cy, 1.25f, { 180, 195, 215, 255 });

    cy += 45.0f;
    std::string s5 = "Created by Prasad Kankhar";
    float w5 = BitmapFont::getTextWidth(s5, 1.4f);
    BitmapFont::drawText(renderer, s5, (Constants::SCREEN_WIDTH - w5) / 2.0f, cy, 1.4f, { 246, 224, 94, 255 });

    cy += 24.0f;
    std::string s6 = "Thank you for playing!";
    float w6 = BitmapFont::getTextWidth(s6, 1.25f);
    BitmapFont::drawText(renderer, s6, (Constants::SCREEN_WIDTH - w6) / 2.0f, cy, 1.25f, { 220, 230, 245, 255 });

    // Buttons
    m_btnEndless.setPosition(85.0f, 480.0f);
    m_btnEndless.setSize(280.0f, 52.0f);
    m_btnEndless.setLabel("PLAY ENDLESS MODE");
    m_btnEndless.render(renderer, { 180, 100, 30, 255 }, { 255, 230, 180, 255 });

    m_btnMenu.setPosition(85.0f, 550.0f);
    m_btnMenu.setSize(280.0f, 50.0f);
    m_btnMenu.setLabel("RETURN TO MAIN MENU");
    m_btnMenu.render(renderer, { 45, 55, 72, 255 });
}

void HUD::renderTutorialHint(SDL_Renderer* renderer, const std::string& hintText, float alpha)
{
    if (hintText.empty() || alpha <= 0.01f) return;

    Uint8 a = static_cast<Uint8>(std::min(alpha, 1.0f) * 230.0f);

    float tw = BitmapFont::getTextWidth(hintText, 1.35f);
    float boxW = tw + 28.0f;
    float boxH = 34.0f;
    float boxX = (Constants::SCREEN_WIDTH - boxW) / 2.0f;
    float boxY = 88.0f;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_FRect box = { boxX, boxY, boxW, boxH };
    SDL_SetRenderDrawColor(renderer, 0x11, 0x14, 0x1d, a);
    SDL_RenderFillRect(renderer, &box);

    SDL_SetRenderDrawColor(renderer, 0xf6, 0xe0, 0x5e, a);
    SDL_RenderRect(renderer, &box);

    BitmapFont::drawText(renderer, hintText, boxX + 14.0f, boxY + 10.0f, 1.35f, { 255, 255, 255, a });
}


