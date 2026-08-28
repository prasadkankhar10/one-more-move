#include "Game.h"
#include "Constants.h"
#include "UI/BitmapFont.h"
#include <iostream>
#include <cmath>

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

    // Load save data
    SaveSystem::load(m_saveData);
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

void Game::startNewGame()
{
    m_currentLevel = m_saveData.highestLevel;
    if (m_currentLevel < 1)
    {
        m_currentLevel = 1;
    }
    m_accumulatedScore = 0;
    m_particles.clear();
    m_worldSystem.reset();
    m_scoreSystem.resetLevel();

    // Seed randomly
    m_currentSeed = static_cast<int>(SDL_GetTicks());
    int px, py, ex, ey;
    m_board = m_generator.generate(m_currentLevel, m_currentSeed, px, py, ex, ey);
    m_player.reset(px, py);

    m_state = GameState::Playing;
}

void Game::loadNextLevel()
{
    m_currentLevel++;
    m_particles.clear();
    m_worldSystem.reset();
    m_scoreSystem.resetLevel();

    m_currentSeed = static_cast<int>(SDL_GetTicks() + m_currentLevel);
    int px, py, ex, ey;
    m_board = m_generator.generate(m_currentLevel, m_currentSeed, px, py, ex, ey);
    m_player.reset(px, py);

    m_state = GameState::Playing;
}

void Game::handleMovement(int dx, int dy)
{
    if (m_state != GameState::Playing || !m_player.isAlive()) return;

    int oldX = m_player.getX();
    int oldY = m_player.getY();

    // Move player logically
    m_player.move(dx, dy, m_board);

    if (m_player.getX() != oldX || m_player.getY() != oldY)
    {
        // Turn count increment
        m_scoreSystem.incrementMoves();
        m_audio.playMoveSound();

        // Update environment turn
        int exitX = -1, exitY = -1;
        // Search board for Exit position
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

        // Check death or win immediately after updating tiles
        TileType landingTile = m_board.getTileType(m_player.getX(), m_player.getY());
        if (landingTile == TileType::Danger)
        {
            triggerGameOver();
        }
        else if (landingTile == TileType::Exit)
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

float Game::getLevelTimeLimit() const
{
    if (m_currentLevel <= 3) return 25.0f;
    if (m_currentLevel <= 6) return 30.0f;
    return 35.0f;
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
    SaveSystem::save(m_saveData);

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
    SaveSystem::save(m_saveData);

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
                startNewGame();
            }
            break;

        case GameState::Playing:
            if (m_hud.getBtnPause().checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::Paused;
            }
            else if (m_hud.getBtnSound().checkClick(mx, my))
            {
                m_audio.toggleSound();
                m_saveData.soundOn = m_audio.isSoundOn();
                SaveSystem::save(m_saveData);
            }
            // D-Pad checks
            else if (m_hud.getBtnUp().checkClick(mx, my))
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
            break;

        case GameState::Paused:
            if (m_hud.m_btnResume.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::Playing;
            }
            else if (m_hud.m_btnMenu.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::MainMenu;
            }
            break;

        case GameState::GameOver:
            if (m_hud.m_btnRestart.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                startNewGame();
            }
            else if (m_hud.m_btnMenu.checkClick(mx, my))
            {
                m_audio.playMoveSound();
                m_state = GameState::MainMenu;
            }
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
                // Restart same level (decrement to reload same level index)
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
        m_scoreSystem.updateTime(deltaTime);
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

    // Render Grid & Entities (Only when Playing, Paused, LevelComplete, or GameOver)
    if (m_state != GameState::MainMenu)
    {
        // Render Board with shake offsets
        m_board.render(m_renderer, shakeX, shakeY);
    }

    // Render Player
    if (m_state == GameState::Playing || m_state == GameState::Paused || m_state == GameState::LevelComplete)
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
            m_hud.renderMainMenu(m_renderer, m_saveData.highScore);
            break;
        case GameState::Playing:
            m_hud.renderPlaying(m_renderer, m_currentLevel, m_scoreSystem.getMoves(), m_accumulatedScore + m_scoreSystem.calculateScore(m_currentLevel, 0.15f), m_audio.isSoundOn());
            
            // Draw Timer HUD
            {
                float limit = getLevelTimeLimit();
                float elapsed = m_scoreSystem.getTime();
                float timeLeft = limit - elapsed;

                if (timeLeft > 0.0f)
                {
                    // Format text
                    char timeStr[32];
                    sprintf(timeStr, "TIME LEFT: %.1fs", timeLeft);
                    
                    // Draw centered time text
                    float scale = 1.5f;
                    float textWidth = strlen(timeStr) * 8.0f * scale;
                    float tx = (Constants::SCREEN_WIDTH - textWidth) / 2.0f;
                    BitmapFont::drawText(m_renderer, timeStr, tx, 80.0f, scale, { 255, 255, 255, 255 });

                    // Draw Timer progress bar
                    float barW = 200.0f;
                    float barH = 6.0f;
                    float bx = (Constants::SCREEN_WIDTH - barW) / 2.0f;
                    float by = 100.0f;

                    // Background bar (dark grey)
                    SDL_FRect bgBar = { bx, by, barW, barH };
                    SDL_SetRenderDrawColor(m_renderer, 0x2d, 0x35, 0x48, 0xff);
                    SDL_RenderFillRect(m_renderer, &bgBar);

                    // Fill bar (green, turns flashing red when low)
                    float percentage = timeLeft / limit;
                    SDL_FRect fillBar = { bx, by, barW * percentage, barH };
                    
                    if (percentage > 0.3f)
                    {
                        SDL_SetRenderDrawColor(m_renderer, 72, 187, 120, 255); // Green
                    }
                    else
                    {
                        // Flashing red
                        Uint8 alpha = (SDL_GetTicks() % 250 < 125) ? 150 : 255;
                        SDL_SetRenderDrawColor(m_renderer, 245, 101, 101, alpha);
                    }
                    SDL_RenderFillRect(m_renderer, &fillBar);
                }
                else
                {
                    // Sudden Death Warning text
                    const char* warnText = "!! SUDDEN DEATH !!";
                    float scale = 1.8f;
                    float textWidth = strlen(warnText) * 8.0f * scale;
                    float tx = (Constants::SCREEN_WIDTH - textWidth) / 2.0f;
                    
                    // Flashing red/yellow text
                    SDL_Color flashColor = (SDL_GetTicks() % 400 < 200) ? SDL_Color{ 245, 101, 101, 255 } : SDL_Color{ 246, 224, 94, 255 };
                    BitmapFont::drawText(m_renderer, warnText, tx, 80.0f, scale, flashColor);
                }
            }
            break;
        case GameState::Paused:
            m_hud.renderPaused(m_renderer);
            break;
        case GameState::GameOver:
            {
                float density = 0.12f + std::min(m_currentLevel * 0.015f, 0.18f);
                int score = m_accumulatedScore + m_scoreSystem.calculateScore(m_currentLevel, density);
                m_hud.renderGameOver(m_renderer, m_currentLevel, score, m_saveData.highScore);
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
