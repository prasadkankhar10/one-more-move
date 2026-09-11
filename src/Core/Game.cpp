#include "Game.h"
#include "Constants.h"
#include "UI/BitmapFont.h"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

Game::Game()
{
}

Game::~Game()
{
    shutdown();
}

bool Game::init()
{
    // Initialize SDL3 Video and Audio
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create Window and Renderer
    m_window = SDL_CreateWindow("One More Move", Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set logical presentation size to 450x800 for automatic letterboxing/scaling
    if (!SDL_SetRenderLogicalPresentation(m_renderer, Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        std::cerr << "Failed to set logical presentation: " << SDL_GetError() << std::endl;
    }

    // Initialize systems
    m_audio.init();
    m_hud.init();

    // Resolve safe pref path for save file (cross-platform storage compliance)
    char* prefPath = SDL_GetPrefPath("GoogleDeepMind", "OneMoreMove");
    if (prefPath)
    {
        m_saveFilePath = std::string(prefPath) + "save_data.txt";
        SDL_free(prefPath);
    }
    else
    {
        m_saveFilePath = "save_data.txt";
    }

    // Load save data
    SaveSystem::load(m_saveData, m_saveFilePath);
    if (!m_saveData.soundOn)
    {
        m_audio.toggleSound();
    }

    m_running = true;
    m_lastTime = SDL_GetTicks();
    m_state = GameState::MainMenu;

    return true;
}

void Game::run()
{
    while (m_running)
    {
        processInput();

        // Calculate delta time
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - m_lastTime) / 1000.0f;
        
        // Prevent huge delta times when dragging window or debug pausing
        if (deltaTime > 0.1f)
        {
            deltaTime = 0.1f;
        }
        
        m_lastTime = currentTime;

        update(deltaTime);
        render();

        // Limit frame rate to ~60 FPS to save power on mobile/laptop
        SDL_Delay(16);
    }
}

void Game::startNewGame(bool fromLevelOne)
{
    if (fromLevelOne)
    {
        m_currentLevel = 1;
    }
    else
    {
        m_currentLevel = m_saveData.highestLevel;
        if (m_currentLevel < 1)
        {
            m_currentLevel = 1;
        }
    }
    m_accumulatedScore = 0;
    m_particles.clear();
    m_worldSystem.reset();
    m_scoreSystem.resetLevel();
    m_reversedTurns = 0;
    m_debuffMessage = "";
    m_debuffMessageTimer = 0.0f;
    m_timedOut = false;
    m_freezeTimer = 0.0f;
    m_lastTime = SDL_GetTicks();

    // Seed randomly
    m_currentSeed = static_cast<int>(SDL_GetTicks());
    int px, py, ex, ey;
    m_board = m_generator.generate(m_currentLevel, m_currentSeed, px, py, ex, ey);
    m_player.reset(px, py);
    m_spawnX = px;
    m_spawnY = py;

    m_state = GameState::Playing;
}

void Game::restartCurrentLevel()
{
    m_particles.clear();
    m_worldSystem.reset();
    m_scoreSystem.resetLevel();
    m_reversedTurns = 0;
    m_debuffMessage = "";
    m_debuffMessageTimer = 0.0f;
    m_timedOut = false;
    m_freezeTimer = 0.0f;
    m_lastTime = SDL_GetTicks();

    // Regenerate the exact same level layout using the stored seed
    int px, py, ex, ey;
    m_board = m_generator.generate(m_currentLevel, m_currentSeed, px, py, ex, ey);
    m_player.reset(px, py);
    m_spawnX = px;
    m_spawnY = py;

    m_state = GameState::Playing;
}

void Game::loadNextLevel()

{
    m_currentLevel++;
    m_particles.clear();
    m_worldSystem.reset();
    m_scoreSystem.resetLevel();
    m_reversedTurns = 0;
    m_debuffMessage = "";
    m_debuffMessageTimer = 0.0f;
    m_timedOut = false;
    m_freezeTimer = 0.0f;
    m_lastTime = SDL_GetTicks();

    m_currentSeed = static_cast<int>(SDL_GetTicks() + m_currentLevel);
    int px, py, ex, ey;
    m_board = m_generator.generate(m_currentLevel, m_currentSeed, px, py, ex, ey);
    m_player.reset(px, py);
    m_spawnX = px;
    m_spawnY = py;

    m_state = GameState::Playing;
}

void Game::handleMovement(int dx, int dy)
{
    if (m_state != GameState::Playing || !m_player.isAlive()) return;

    // Apply Reversed Controls Debuff
    if (m_reversedTurns > 0)
    {
        dx = -dx;
        dy = -dy;
    }

    int oldX = m_player.getX();
    int oldY = m_player.getY();
    TileType prevStepType = m_board.getTileType(oldX, oldY);

    int targetX = oldX + dx;
    int targetY = oldY + dy;

    // Check Gate tile before moving
    if (m_board.isValidPosition(targetX, targetY) && m_board.getTileType(targetX, targetY) == TileType::Gate)
    {
        if (m_player.hasKey())
        {
            // Unlock and open gate
            m_player.setKey(false);
            m_board.setTileType(targetX, targetY, TileType::Empty);
            m_audio.playGateSound();
            m_debuffMessage = "! GATE UNLOCKED !";
            m_debuffMessageTimer = 2.0f;

            float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
            float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
            float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
            float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;
            spawnExplosion(originX + targetX * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                           originY + targetY * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                           { 250, 204, 21, 255 });
        }
        else
        {
            m_audio.playInvalidMoveSound();
            m_debuffMessage = "! GATE LOCKED (FIND KEY) !";
            m_debuffMessageTimer = 2.0f;
            return;
        }
    }

    // Move player logically
    m_player.move(dx, dy, m_board);

    if (m_player.getX() != oldX || m_player.getY() != oldY)
    {
        // Turn count increment
        m_scoreSystem.incrementMoves();
        m_audio.playMoveSound();

        // If previous tile was Crumbling, it collapses into an impassable Pit!
        if (prevStepType == TileType::Crumbling)
        {
            m_board.setTileType(oldX, oldY, TileType::Pit);
            m_audio.playDestructionSound();

            float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
            float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
            float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
            float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;
            spawnExplosion(originX + oldX * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                           originY + oldY * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                           { 214, 158, 46, 255 });
        }

        // Decrement reversed controls counter on successful move
        if (m_reversedTurns > 0)
        {
            m_reversedTurns--;
        }

        // Ice slide mechanic: continue sliding until hitting non-ice or obstacle
        if (m_board.getTileType(m_player.getX(), m_player.getY()) == TileType::Ice)
        {
            m_audio.playSlideSound();
            while (m_board.getTileType(m_player.getX(), m_player.getY()) == TileType::Ice)
            {
                int nextSlideX = m_player.getX() + dx;
                int nextSlideY = m_player.getY() + dy;
                if (!m_board.isValidPosition(nextSlideX, nextSlideY)) break;

                TileType nxtType = m_board.getTileType(nextSlideX, nextSlideY);
                if (nxtType == TileType::Wall || (nxtType == TileType::Gate && !m_player.hasKey())) break;

                // Advance one step forward on ice
                m_player.setPosition(nextSlideX, nextSlideY);

                // Stop if landed on a non-ice tile (e.g. Empty, Hazard, Portal, Exit)
                if (nxtType != TileType::Ice) break;
            }
        }

        // Process landing tile effects
        TileType landingTile = m_board.getTileType(m_player.getX(), m_player.getY());

        // 1. Portal Teleportation
        if (landingTile == TileType::Portal)
        {
            const Tile& pTile = m_board.getTile(m_player.getX(), m_player.getY());
            if (m_board.isValidPosition(pTile.portalTargetX, pTile.portalTargetY))
            {
                m_player.setPosition(pTile.portalTargetX, pTile.portalTargetY);
                m_audio.playPortalSound();

                float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
                float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
                float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
                float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;
                spawnExplosion(originX + pTile.portalTargetX * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                               originY + pTile.portalTargetY * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                               { 183, 148, 244, 255 });
            }
        }
        // 2. Key Pickup
        else if (landingTile == TileType::Key)
        {
            m_player.setKey(true);
            m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
            m_audio.playKeySound();
            m_debuffMessage = "! KEY ACQUIRED !";
            m_debuffMessageTimer = 2.0f;
        }
        // 3. Shield Power-up
        else if (landingTile == TileType::Shield)
        {
            m_player.setShield(true);
            m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
            m_audio.playShieldSound();
            m_debuffMessage = "! ENERGY SHIELD EQUIPPED !";
            m_debuffMessageTimer = 2.5f;
        }
        // 4. Time Freeze Power-up
        else if (landingTile == TileType::TimeFreeze)
        {
            m_freezeTimer = 8.0f;
            m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
            m_audio.playFreezeSound();
            m_debuffMessage = "! TIME FROZEN (8 SECONDS) !";
            m_debuffMessageTimer = 2.5f;
        }
        // 5. Coin Collectible
        else if (landingTile == TileType::Coin)
        {
            m_accumulatedScore += 500;
            m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
            m_audio.playCoinSound();
            m_debuffMessage = "+500 COIN BONUS!";
            m_debuffMessageTimer = 1.8f;
        }
        // 6. Bomb Detonation
        else if (landingTile == TileType::Bomb)
        {
            detonateBomb(m_player.getX(), m_player.getY());
        }
        // 7. Curse Tile
        else if (landingTile == TileType::Curse)
        {
            if (m_player.hasShield())
            {
                m_player.setShield(false);
                m_audio.playShieldSound();
                m_debuffMessage = "! SHIELD ABSORBED CURSE !";
                m_debuffMessageTimer = 2.5f;
                m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
            }
            else
            {
                DebuffType debuff = m_board.getDebuffType(m_player.getX(), m_player.getY());
                triggerDebuff(debuff);
                m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
            }
        }
        // 8. Defuse Tile
        else if (landingTile == TileType::Defuse)
        {
            m_reversedTurns = 0;
            m_debuffMessage = "! HAZARDS DEFUSED !";
            m_debuffMessageTimer = 2.5f;
            m_audio.playWinSound();

            for (int ny = m_player.getY() - 1; ny <= m_player.getY() + 1; ++ny)
            {
                for (int nx = m_player.getX() - 1; nx <= m_player.getX() + 1; ++nx)
                {
                    if (m_board.isValidPosition(nx, ny) && m_board.getTileType(nx, ny) == TileType::Danger)
                    {
                        m_board.setTileType(nx, ny, TileType::Empty);
                        spawnExplosion(nx * 40.0f, ny * 40.0f, { 56, 178, 172, 255 });
                    }
                }
            }
            m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
        }
        // 9. Pit (Bottomless pit)
        else if (landingTile == TileType::Pit)
        {
            if (m_player.hasShield())
            {
                m_player.setShield(false);
                m_audio.playShieldSound();
                m_debuffMessage = "! SHIELD SAVED YOU FROM PIT !";
                m_debuffMessageTimer = 2.5f;
            }
            else
            {
                m_timedOut = false;
                triggerGameOver();
                return;
            }
        }

        // Update environment turn
        int exitX = -1, exitY = -1;
        for (int y = 0; y < m_board.getHeight(); ++y)
        {
            for (int x = 0; x < m_board.getWidth(); ++x)
            {
                if (m_board.getTileType(x, y) == TileType::Exit)
                {
                    exitX = x;
                    exitY = y;
                    break;
                }
            }
        }

        m_worldSystem.updateWorld(m_board, m_player.getX(), m_player.getY(), exitX, exitY,
                                   m_scoreSystem.getMoves(), m_currentLevel, m_audio);

        // Check if Sudden Death time limit is exceeded
        if (m_scoreSystem.getTime() > getLevelTimeLimit())
        {
            m_worldSystem.triggerSuddenDeathCollapse(m_board, m_player.getX(), m_player.getY(), exitX, exitY, m_audio);
        }

        // Re-check death or win after world hazards update
        TileType currentTile = m_board.getTileType(m_player.getX(), m_player.getY());
        if (currentTile == TileType::Danger || currentTile == TileType::Pit)
        {
            if (m_player.hasShield())
            {
                m_player.setShield(false);
                m_audio.playShieldSound();
                m_debuffMessage = "! SHIELD ABSORBED HAZARD !";
                m_debuffMessageTimer = 2.5f;
                if (currentTile == TileType::Danger)
                {
                    m_board.setTileType(m_player.getX(), m_player.getY(), TileType::Empty);
                }
            }
            else
            {
                m_timedOut = false;
                triggerGameOver();
            }
        }
        else if (currentTile == TileType::Exit)
        {
            triggerLevelComplete();
        }
    }
    else
    {
        // Blocked movement
        m_audio.playInvalidMoveSound();
    }
}

void Game::triggerDebuff(DebuffType debuff)
{
    m_audio.playWarningSound();
    m_shakeTime = 0.4f;
    m_shakeMagnitude = 9.0f;

    switch (debuff)
    {
        case DebuffType::ReverseControls:
            m_reversedTurns = 4;
            m_debuffMessage = "! CONTROLS REVERSED (4 MOVES) !";
            m_debuffMessageTimer = 3.0f;
            break;

        case DebuffType::TeleportSpawn:
            m_player.reset(m_spawnX, m_spawnY);
            m_debuffMessage = "! TELEPORTED TO SPAWN !";
            m_debuffMessageTimer = 3.0f;
            break;

        case DebuffType::ReviseMap:
            reviseMap();
            m_debuffMessage = "! MAP RESHUFFLED !";
            m_debuffMessageTimer = 3.0f;
            break;

        case DebuffType::TimePenalty:
            m_scoreSystem.updateTime(10.0f); // Adds 10s to elapsed time = 10s penalty
            m_debuffMessage = "! -10s TIME PENALTY !";
            m_debuffMessageTimer = 3.0f;
            break;

        case DebuffType::None:
            break;
    }
}

void Game::reviseMap()
{
    int exitX = -1, exitY = -1;
    for (int y = 0; y < m_board.getHeight(); ++y)
    {
        for (int x = 0; x < m_board.getWidth(); ++x)
        {
            if (m_board.getTileType(x, y) == TileType::Exit)
            {
                exitX = x;
                exitY = y;
                break;
            }
        }
    }

    std::vector<std::pair<int, int>> positions;
    std::vector<TileType> tileTypes;
    for (int y = 1; y < m_board.getHeight() - 1; ++y)
    {
        for (int x = 1; x < m_board.getWidth() - 1; ++x)
        {
            if ((x == m_player.getX() && y == m_player.getY()) || (x == exitX && y == exitY))
            {
                continue;
            }
            TileType t = m_board.getTileType(x, y);
            if (t == TileType::Wall || t == TileType::Empty || t == TileType::Danger)
            {
                positions.push_back({x, y});
                tileTypes.push_back(t);
            }
        }
    }

    if (positions.empty()) return;

    std::mt19937 rng(static_cast<unsigned int>(SDL_GetTicks()));
    for (int attempt = 0; attempt < 25; ++attempt)
    {
        std::shuffle(tileTypes.begin(), tileTypes.end(), rng);
        for (size_t i = 0; i < positions.size(); ++i)
        {
            m_board.setTileType(positions[i].first, positions[i].second, tileTypes[i]);
        }

        if (m_board.hasPath(m_player.getX(), m_player.getY(), exitX, exitY))
        {
            return;
        }
    }

    // Fallback: carve guaranteed clear corridor if shuffles fail
    int cx = m_player.getX();
    int cy = m_player.getY();
    while (cx != exitX)
    {
        cx += (exitX > cx) ? 1 : -1;
        if (m_board.getTileType(cx, cy) == TileType::Wall)
        {
            m_board.setTileType(cx, cy, TileType::Empty);
        }
    }
    while (cy != exitY)
    {
        cy += (exitY > cy) ? 1 : -1;
        if (m_board.getTileType(cx, cy) == TileType::Wall)
        {
            m_board.setTileType(cx, cy, TileType::Empty);
        }
    }
}

void Game::detonateBomb(int bx, int by)
{
    m_audio.playBombSound();
    m_shakeTime = 0.5f;
    m_shakeMagnitude = 14.0f;
    m_debuffMessage = "! BOMB BLAST (3x3 CLEARED) !";
    m_debuffMessageTimer = 2.5f;

    float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
    float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
    float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
    float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;

    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int nx = bx + dx;
            int ny = by + dy;

            // Do not destroy outer border walls
            if (nx <= 0 || nx >= m_board.getWidth() - 1 || ny <= 0 || ny >= m_board.getHeight() - 1)
            {
                continue;
            }

            TileType t = m_board.getTileType(nx, ny);
            if (t != TileType::Exit)
            {
                m_board.setTileType(nx, ny, TileType::Empty);
                spawnExplosion(originX + nx * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                               originY + ny * (Constants::TILE_SIZE + Constants::GRID_SPACING) + 20.0f,
                               { 245, 101, 101, 255 });
            }
        }
    }
}

float Game::getLevelTimeLimit() const
{
    float pathSteps = static_cast<float>(m_board.getOptimalPath().size());
    if (pathSteps < 6.0f) pathSteps = 6.0f;

    // Guaranteed time budget: 1.4 seconds per optimal step + 7.0 seconds buffer
    float calculated = (pathSteps * 1.4f) + 7.0f;
    float minLimit = (m_currentLevel <= 3) ? 22.0f : ((m_currentLevel <= 6) ? 28.0f : 32.0f);
    return std::max(calculated, minLimit);
}


void Game::triggerGameOver()
{
    m_player.setAlive(false);
    m_audio.playDeathSound();
    m_shakeTime = 0.5f;
    m_shakeMagnitude = 12.0f;

    // Get screen position of death for explosion
    float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
    float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
    float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
    float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;

    float px = originX + m_player.getVisualX() * (Constants::TILE_SIZE + Constants::GRID_SPACING) + Constants::TILE_SIZE / 2.0f;
    float py = originY + m_player.getVisualY() * (Constants::TILE_SIZE + Constants::GRID_SPACING) + Constants::TILE_SIZE / 2.0f;

    spawnExplosion(px, py, { 245, 101, 101, 255 }); // Red explosion

    // Calculate score
    float density = 0.12f + std::min(m_currentLevel * 0.015f, 0.18f);
    int levelScore = m_scoreSystem.calculateScore(m_currentLevel, density);
    int finalScore = m_accumulatedScore + levelScore;

    // Save progression
    if (finalScore > m_saveData.highScore)
    {
        m_saveData.highScore = finalScore;
    }
    if (m_currentLevel > m_saveData.highestLevel)
    {
        m_saveData.highestLevel = m_currentLevel;
    }
    SaveSystem::save(m_saveData, m_saveFilePath);

    m_state = GameState::GameOver;
}

void Game::triggerLevelComplete()
{
    m_audio.playWinSound();

    // Get screen position of exit for explosion sparks
    float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
    float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
    float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
    float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;

    float ex = originX + m_player.getX() * (Constants::TILE_SIZE + Constants::GRID_SPACING) + Constants::TILE_SIZE / 2.0f;
    float ey = originY + m_player.getY() * (Constants::TILE_SIZE + Constants::GRID_SPACING) + Constants::TILE_SIZE / 2.0f;

    spawnExplosion(ex, ey, { 72, 187, 120, 255 }); // Green stars explosion

    // Calculate level score
    float density = 0.12f + std::min(m_currentLevel * 0.015f, 0.18f);
    int levelScore = m_scoreSystem.calculateScore(m_currentLevel, density);
    m_accumulatedScore += levelScore;

    // Save progress
    if (m_accumulatedScore > m_saveData.highScore)
    {
        m_saveData.highScore = m_accumulatedScore;
    }
    if (m_currentLevel + 1 > m_saveData.highestLevel)
    {
        m_saveData.highestLevel = m_currentLevel + 1;
    }
    SaveSystem::save(m_saveData, m_saveFilePath);

    m_state = GameState::LevelComplete;
}

void Game::spawnExplosion(float x, float y, SDL_Color color)
{
    for (int i = 0; i < 45; ++i)
    {
        Particle p;
        p.x = x;
        p.y = y;
        
        // Random explosion velocities
        float angle = (rand() % 360) * (3.14159f / 180.0f);
        float speed = 50.0f + (rand() % 250);
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;

        p.life = 0.3f + (rand() % 40) / 100.0f;
        p.maxLife = p.life;
        p.color = color;
        p.size = 4.0f + rand() % 5;

        m_particles.push_back(p);
    }
}

void Game::handleMouseClick(float mx, float my)
{
    switch (m_state)
    {
        case GameState::MainMenu:
            if (m_hud.m_btnPlay.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                startNewGame(false);
            }
            else if (m_hud.m_btnInfo.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_infoTab = 0;
                m_state = GameState::Info;
            }
            else if (m_hud.getBtnControls().checkClick(mx, my))
            {
                m_saveData.controlMode = (m_saveData.controlMode + 1) % 3;
                SaveSystem::save(m_saveData, m_saveFilePath);
                m_audio.playMoveSound();
            }
            else if (m_hud.getBtnExit().checkClick(mx, my))
            {
                m_running = false;
            }
            break;

        case GameState::Playing:
            if (m_hud.m_btnGiveUp.checkClick(mx, my))
            {
                m_audio.playDeathSound();
                m_timedOut = false;
                triggerGameOver();
            }
            else if (m_hud.m_btnRestartRun.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                startNewGame(true);
            }
            else if (m_hud.getBtnControls().checkClick(mx, my))
            {
                m_saveData.controlMode = (m_saveData.controlMode + 1) % 3;
                SaveSystem::save(m_saveData, m_saveFilePath);
                m_audio.playMoveSound();
            }
            else if (m_hud.getBtnPause().checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::Paused;
            }
            else if (m_hud.getBtnSound().checkClick(mx, my))
            {
                m_audio.toggleSound();
                m_saveData.soundOn = m_audio.isSoundOn();
                SaveSystem::save(m_saveData, m_saveFilePath);
            }
            // D-Pad checks (Only enabled if controlMode is Both or DPadOnly)
            else if (m_saveData.controlMode != 1)
            {
                if (m_hud.getBtnUp().checkClick(mx, my))
                {
                    handleMovement(0, -1);
                }
                else if (m_hud.getBtnDown().checkClick(mx, my))
                {
                    handleMovement(0, 1);
                }
                else if (m_hud.getBtnLeft().checkClick(mx, my))
                {
                    handleMovement(-1, 0);
                }
                else if (m_hud.getBtnRight().checkClick(mx, my))
                {
                    handleMovement(1, 0);
                }
            }
            break;

        case GameState::Paused:
            if (m_hud.m_btnResume.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::Playing;
            }
            else if (m_hud.m_btnGiveUp.checkClick(mx, my))
            {
                m_audio.playDeathSound();
                m_timedOut = false;
                triggerGameOver();
            }
            else if (m_hud.m_btnRestartRun.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                startNewGame(true);
            }
            else if (m_hud.m_btnMenu.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::MainMenu;
            }
            else if (m_hud.getBtnExit().checkClick(mx, my))
            {
                m_running = false;
            }
            break;

        case GameState::Info:
            if (m_hud.m_btnTabPlay.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_infoTab = 0;
            }
            else if (m_hud.m_btnTabTiles.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_infoTab = 1;
            }
            else if (m_hud.m_btnTabDev.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_infoTab = 2;
            }
            else if (m_hud.m_btnBack.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::MainMenu;
            }
            break;

        case GameState::GameOver:
            if (m_hud.m_btnRestart.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                restartCurrentLevel();
            }
            else if (m_hud.m_btnShowPath.checkClick(mx, my))
            {
                m_audio.playWinSound();
                m_state = GameState::PathPreview;
            }
            else if (m_hud.m_btnMenu.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::MainMenu;
            }
            break;

        case GameState::PathPreview:
            // Tapping anywhere or clicking retry challenge restarts the level
            m_audio.playMoveSound();
            restartCurrentLevel();
            break;


        case GameState::LevelComplete:
            if (m_hud.m_btnNext.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                loadNextLevel();
            }
            else if (m_hud.m_btnRestart.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                // Restart same level
                m_currentLevel--;
                loadNextLevel();
            }
            else if (m_hud.m_btnMenu.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::MainMenu;
            }
            break;
    }
}

void Game::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            m_running = false;
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            handleMouseClick(event.button.x, event.button.y);
        }
        else if (event.type == SDL_EVENT_FINGER_DOWN)
        {
            int winW = 0, winH = 0;
            SDL_GetWindowSize(m_window, &winW, &winH);
            
            float wx = event.tfinger.x * static_cast<float>(winW);
            float wy = event.tfinger.y * static_cast<float>(winH);
            
            float rx = 0.0f, ry = 0.0f;
            if (SDL_RenderCoordinatesFromWindow(m_renderer, wx, wy, &rx, &ry))
            {
                m_touchStartX = rx;
                m_touchStartY = ry;
                m_isSwiping = true;
            }
        }
        else if (event.type == SDL_EVENT_FINGER_UP)
        {
            if (m_isSwiping)
            {
                m_isSwiping = false;
                int winW = 0, winH = 0;
                SDL_GetWindowSize(m_window, &winW, &winH);

                float wx = event.tfinger.x * static_cast<float>(winW);
                float wy = event.tfinger.y * static_cast<float>(winH);

                float rx = 0.0f, ry = 0.0f;
                if (SDL_RenderCoordinatesFromWindow(m_renderer, wx, wy, &rx, &ry))
                {
                    float deltaX = rx - m_touchStartX;
                    float deltaY = ry - m_touchStartY;
                    float distSq = deltaX * deltaX + deltaY * deltaY;

                    // Swipe threshold: 30 pixels (900 px squared)
                    if (distSq >= 900.0f && m_state == GameState::Playing && (m_saveData.controlMode == 0 || m_saveData.controlMode == 1))
                    {
                        if (std::abs(deltaX) > std::abs(deltaY))
                        {
                            handleMovement((deltaX > 0.0f) ? 1 : -1, 0);
                        }
                        else
                        {
                            handleMovement(0, (deltaY > 0.0f) ? 1 : -1);
                        }
                    }
                    else
                    {
                        // Process as button tap / click
                        handleMouseClick(rx, ry);
                    }
                }
            }
        }
        else if (event.type == SDL_EVENT_KEY_UP)
        {
            m_keyReleased = true;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                if (m_state == GameState::Playing) m_state = GameState::Paused;
                else if (m_state == GameState::Paused) m_state = GameState::Playing;
                else if (m_state == GameState::MainMenu) m_running = false;
            }

            // Keyboard movements: block repeating inputs to prevent sliding
            if (m_state == GameState::Playing && m_keyReleased)
            {
                m_keyReleased = false;

                if (event.key.key == SDLK_UP || event.key.key == SDLK_W)
                {
                    handleMovement(0, -1);
                }
                else if (event.key.key == SDLK_DOWN || event.key.key == SDLK_S)
                {
                    handleMovement(0, 1);
                }
                else if (event.key.key == SDLK_LEFT || event.key.key == SDLK_A)
                {
                    handleMovement(-1, 0);
                }
                else if (event.key.key == SDLK_RIGHT || event.key.key == SDLK_D)
                {
                    handleMovement(1, 0);
                }

                // Keyboard cheat/debug hotkeys
                else if (event.key.key == SDLK_R) // Regenerate level
                {
                    m_currentLevel--;
                    loadNextLevel();
                }
                else if (event.key.key == SDLK_N) // Skip to next level
                {
                    loadNextLevel();
                }
                else if (event.key.key == SDLK_K) // Kill player
                {
                    triggerGameOver();
                }
                else if (event.key.key == SDLK_V) // Win level
                {
                    triggerLevelComplete();
                }
            }
        }
    }
}

void Game::update(float deltaTime)
{
    // Update screen shake decay
    if (m_shakeTime > 0.0f)
    {
        m_shakeTime -= deltaTime;
        if (m_shakeTime < 0.0f) m_shakeTime = 0.0f;
    }

    // Update debuff message timer
    if (m_debuffMessageTimer > 0.0f)
    {
        m_debuffMessageTimer -= deltaTime;
        if (m_debuffMessageTimer <= 0.0f)
        {
            m_debuffMessage = "";
        }
    }

    // Update and filter explosion particles
    for (auto it = m_particles.begin(); it != m_particles.end();)
    {
        it->x += it->vx * deltaTime;
        it->y += it->vy * deltaTime;
        it->life -= deltaTime;

        if (it->life <= 0.0f)
        {
            it = m_particles.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Update Playing entities
    if (m_state == GameState::Playing)
    {
        m_player.update(deltaTime);

        if (m_freezeTimer > 0.0f)
        {
            m_freezeTimer -= deltaTime;
            if (m_freezeTimer < 0.0f) m_freezeTimer = 0.0f;
        }
        else
        {
            m_scoreSystem.updateTime(deltaTime);

            // Fail level immediately if level countdown timer hits zero!
            if (m_scoreSystem.getTime() >= getLevelTimeLimit())
            {
                m_timedOut = true;
                triggerGameOver();
            }
        }

        // Procedural 8-bit dynamic chiptune background music
        bool isUrgent = (getLevelTimeLimit() - m_scoreSystem.getTime() < 5.0f && m_freezeTimer <= 0.0f);
        m_audio.updateBGM(isUrgent, m_state == GameState::Playing);
    }
    else
    {
        m_audio.stopBGM();
    }
}

void Game::render()
{
    // Clear back buffer to deep slate grey/blue (#1a1e29)
    SDL_SetRenderDrawColor(m_renderer, 0x1a, 0x1e, 0x29, 0xff);
    SDL_RenderClear(m_renderer);

    // Calculate screen shake offsets
    float shakeX = 0.0f;
    float shakeY = 0.0f;
    if (m_shakeTime > 0.0f)
    {
        // Generates random directional vectors scaled by current magnitude
        shakeX = ((rand() % 200 - 100) / 100.0f) * m_shakeMagnitude;
        shakeY = ((rand() % 200 - 100) / 100.0f) * m_shakeMagnitude;
    }

    // Centering calculations for grid elements
    float totalWidth = m_board.getWidth() * Constants::TILE_SIZE + (m_board.getWidth() - 1) * Constants::GRID_SPACING;
    float totalHeight = m_board.getHeight() * Constants::TILE_SIZE + (m_board.getHeight() - 1) * Constants::GRID_SPACING;
    float originX = (Constants::SCREEN_WIDTH - totalWidth) / 2.0f;
    float originY = (Constants::SCREEN_HEIGHT - totalHeight) / 2.0f;

    // Apply screen shake to grid layout rendering
    float renderOriginX = originX + shakeX;
    float renderOriginY = originY + shakeY;

    // Render Grid & Entities (Only when Playing, Paused, LevelComplete, GameOver, or PathPreview)
    if (m_state != GameState::MainMenu)
    {
        // Render Board with shake offsets
        m_board.render(m_renderer, shakeX, shakeY);
    }

    // Render Path Preview trail when player requests solution
    if (m_state == GameState::PathPreview)
    {
        m_board.renderOptimalPath(m_renderer);
    }

    // Render Player
    if (m_state == GameState::Playing || m_state == GameState::Paused || m_state == GameState::LevelComplete || m_state == GameState::PathPreview)
    {
        m_player.render(m_renderer, renderOriginX, renderOriginY);
    }


    // Render Particles
    for (const auto& p : m_particles)
    {
        SDL_FRect r = { p.x + shakeX, p.y + shakeY, p.size, p.size };
        SDL_SetRenderDrawColor(m_renderer, p.color.r, p.color.g, p.color.b, p.color.a);
        SDL_RenderFillRect(m_renderer, &r);
    }

    // Render HUD and Overlays (Static, no shake)
    switch (m_state)
    {
        case GameState::MainMenu:
            m_hud.renderMainMenu(m_renderer, m_saveData.highScore, m_saveData.highestLevel, m_saveData.controlMode);
            break;
        case GameState::Playing:
            {
                float limit = getLevelTimeLimit();
                float elapsed = m_scoreSystem.getTime();
                float timeLeft = limit - elapsed;

                m_hud.renderPlaying(m_renderer, m_currentLevel, m_scoreSystem.getMoves(),
                                    m_accumulatedScore + m_scoreSystem.calculateScore(m_currentLevel, 0.15f),
                                    m_audio.isSoundOn(), timeLeft, limit, m_saveData.controlMode,
                                    m_reversedTurns, m_debuffMessage,
                                    m_player.hasShield(), m_player.hasKey(), m_freezeTimer, m_board.getBiomeName());
            }
            break;
        case GameState::Paused:
            m_hud.renderPaused(m_renderer);
            break;
        case GameState::GameOver:
            {
                float density = 0.12f + std::min(m_currentLevel * 0.015f, 0.18f);
                int score = m_accumulatedScore + m_scoreSystem.calculateScore(m_currentLevel, density);
                m_hud.renderGameOver(m_renderer, m_currentLevel, score, m_saveData.highScore, m_timedOut);
            }
            break;
        case GameState::LevelComplete:
            {
                float density = 0.12f + std::min(m_currentLevel * 0.015f, 0.18f);
                int score = m_accumulatedScore; // Already accumulated in triggerLevelComplete
                int stars = m_scoreSystem.calculateStars(m_currentLevel);
                m_hud.renderLevelComplete(m_renderer, m_currentLevel, score, m_scoreSystem.getMoves(), m_scoreSystem.getTime(), stars, m_saveData.highScore);
            }
            break;
        case GameState::Info:
            m_hud.renderInfo(m_renderer, m_infoTab);
            break;
        case GameState::PathPreview:
            {
                int pathMoves = static_cast<int>(m_board.getOptimalPath().size());
                if (pathMoves > 0) pathMoves -= 1;
                m_hud.renderPathPreview(m_renderer, pathMoves);
            }
            break;
    }


    // Present rendering
    SDL_RenderPresent(m_renderer);
}

void Game::shutdown()
{
    m_audio.shutdown();

    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}
