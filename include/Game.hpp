#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <queue>
#include <random>
#include "Player.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "Leaderboard.hpp"
#include "AudioManager.hpp"
#include "VisualEffects.hpp"

enum class GameState {
    UsernameEntry,
    MainMenu,
    Playing,
    Paused,
    Leaderboard,
    Controls,
    GameOver
};

/**
 * @brief Core Game class managing the lifecycle, window, event processing,
 *        and rendering pipeline for Space Defender.
 */
class Game {
public:
    static constexpr unsigned int WINDOW_WIDTH = 1000;
    static constexpr unsigned int WINDOW_HEIGHT = 700;
    static constexpr const char* WINDOW_TITLE = "Space Defender";
    static constexpr float SHOOT_COOLDOWN_SECONDS = 0.25f;

    /**
     * @brief Constructor initializing the window and core systems.
     * @param testMode If true, runs for a limited number of frames for verification.
     */
    explicit Game(bool testMode = false);
    ~Game() = default;

    // Prevent copying to avoid multiple window handles
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Allow moving if needed
    Game(Game&&) noexcept = default;
    Game& operator=(Game&&) noexcept = default;

    /**
     * @brief Starts the main game loop.
     */
    void run();

    /**
     * @brief Checks if the game window is currently open.
     */
    [[nodiscard]] bool isRunning() const;

    // State & Testing Accessors
    [[nodiscard]] GameState getGameState() const { return m_gameState; }
    void setGameState(GameState state) { m_gameState = state; }

    [[nodiscard]] const std::string& getUsername() const { return m_playerNameInput; }
    void setUsername(const std::string& name) { m_playerNameInput = name; }

    [[nodiscard]] const std::string& getValidationError() const { return m_usernameValidationError; }
    [[nodiscard]] bool isScoreSubmitted() const { return m_scoreSubmitted; }

    void simulateTextInput(sf::Uint32 unicode) { handleTextInput(unicode); }
    void simulateGameOver() { triggerGameOver(); }
    void restartMission() { resetGame(); }

    [[nodiscard]] int getScore() const { return m_score; }
    void setScore(int score) { m_score = score; }

    [[nodiscard]] const Leaderboard& getLeaderboard() const { return m_leaderboard; }

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render();

    void fireBullet();
    void updateBullets(sf::Time deltaTime);

    void initWaves();
    void updateWaves(sf::Time deltaTime);
    void spawnEnemy();
    void updateEnemies(sf::Time deltaTime);
    void updateCollisions();
    void triggerGameOver();

    void initHUD();
    void updateHUD();
    void renderHUD();
    void renderGameOver();
    void renderUsernameEntry();
    void renderLeaderboard();
    void renderMainMenu();
    void renderPaused();
    void renderControls();
    void handleTextInput(sf::Uint32 unicode);
    void resetGame();
    void runSortingBenchmark();

    void updateWindowTitle();

    sf::RenderWindow m_window;
    Player m_player;
    std::vector<Bullet> m_bullets;
    std::vector<Enemy> m_enemies;
    std::queue<int> m_enemyWaves;

    int m_score;
    int m_currentWave;
    int m_enemiesRemainingToSpawn;
    int m_totalEnemiesDefeated;
    bool m_isGameOver;
    GameState m_gameState;
    GameState m_previousState;

    // Leaderboard & Username
    Leaderboard m_leaderboard;
    std::string m_playerNameInput;
    bool m_scoreSubmitted;
    std::string m_usernameValidationError;
    AlgorithmMetrics m_lastMetrics;

    // Audio & Visual Effects
    AudioManager m_audio;
    VisualEffects m_vfx;

    int m_menuSelectedIndex;
    int m_pauseSelectedIndex;

    sf::Clock m_shootClock;
    sf::Time m_shootCooldown;

    sf::Clock m_enemySpawnClock;
    sf::Time m_enemySpawnInterval;

    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_xDist;

    // HUD & Font Assets
    sf::Font m_font;
    bool m_fontLoaded;
    sf::Text m_scoreText;
    sf::Text m_waveText;
    sf::Text m_healthText;

    // Username Entry UI Elements
    sf::Text m_usernameTitleText;
    sf::Text m_usernameSubtitleText;
    sf::Text m_usernamePromptText;
    sf::Text m_usernameInputText;
    sf::Text m_usernameLimitText;
    sf::Text m_usernameInstructionText;
    sf::Text m_usernameValidationText;
    sf::RectangleShape m_usernameInputBox;
    sf::Clock m_cursorClock;

    // Game Over UI Elements
    sf::Text m_gameOverText;
    sf::Text m_finalScoreText;
    sf::Text m_gameOverUserText;
    sf::Text m_gameOverSavedText;
    sf::Text m_instructionText;

    // Leaderboard Screen – readable monospaced font (separate from m_font)
    sf::Font m_leaderboardFont;
    bool     m_leaderboardFontLoaded;

    // Leaderboard Screen Text Assets
    sf::Text m_leaderboardHeaderText;
    sf::Text m_leaderboardAlgoText;       // kept for bookkeeping; NOT drawn
    sf::Text m_leaderboardColumnText;     // column heading row
    sf::Text m_leaderboardMetricsText;
    sf::Text m_leaderboardRowsText;
    sf::Text m_leaderboardControlsText;

    // Menu & Pause Text Assets
    sf::Text m_menuTitleText;
    sf::Text m_menuSubtitleText;
    sf::Text m_menuOptionsText[4];
    sf::Text m_pauseTitleText;
    sf::Text m_pauseOptionsText[3];
    sf::Text m_controlsTitleText;
    sf::Text m_controlsBodyText;
    sf::Text m_controlsPromptText;

    sf::RectangleShape m_healthBarBg;
    sf::RectangleShape m_healthBarFill;
    sf::RectangleShape m_gameOverOverlay;

    bool m_testMode;
    unsigned int m_frameCount;
};
