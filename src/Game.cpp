#include "Game.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>

Game::Game(bool testMode)
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE, sf::Style::Titlebar | sf::Style::Close)
    , m_player(static_cast<float>(WINDOW_WIDTH) / 2.0f, static_cast<float>(WINDOW_HEIGHT) - 80.0f)
    , m_score(0)
    , m_currentWave(0)
    , m_enemiesRemainingToSpawn(0)
    , m_totalEnemiesDefeated(0)
    , m_isGameOver(false)
    , m_gameState(testMode ? GameState::Playing : GameState::UsernameEntry)
    , m_previousState(GameState::MainMenu)
    , m_leaderboard("data/leaderboard.txt")
    , m_playerNameInput(testMode ? "TEST_PILOT" : "")
    , m_scoreSubmitted(false)
    , m_usernameValidationError("")
    , m_menuSelectedIndex(0)
    , m_pauseSelectedIndex(0)
    , m_shootCooldown(sf::seconds(SHOOT_COOLDOWN_SECONDS))
    , m_enemySpawnInterval(sf::seconds(1.2f))
    , m_rng(std::random_device{}())
    , m_xDist(Enemy::ENEMY_WIDTH / 2.0f, static_cast<float>(WINDOW_WIDTH) - Enemy::ENEMY_WIDTH / 2.0f)
    , m_fontLoaded(false)
    , m_testMode(testMode)
    , m_frameCount(0)
{
    m_window.setFramerateLimit(60);

    // Initial audio and VFX setup
    m_vfx.init(WINDOW_WIDTH, WINDOW_HEIGHT);
    m_audio.init();

    // Initial sort of loaded leaderboard entries
    m_leaderboard.insertionSort(&m_lastMetrics);

    initHUD();
    initWaves();
    updateHUD();
    updateWindowTitle();

    std::cout << "[Space Defender] Game initialized." << std::endl;
    std::cout << "[Space Defender] Window created: " << WINDOW_WIDTH << "x" << WINDOW_HEIGHT
              << " | Title: '" << WINDOW_TITLE << "'" << std::endl;
}

void Game::initHUD() {
    // 1. Load Font (Prioritize local SIL OFL arcade font, fallback to system TrueType)
    if (m_font.loadFromFile("assets/fonts/PressStart2P.ttf")) {
        m_fontLoaded = true;
        std::cout << "[Space Defender] Loaded arcade font: assets/fonts/PressStart2P.ttf" << std::endl;
    } else if (m_font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        m_fontLoaded = true;
        std::cout << "[Space Defender] Fallback font loaded: C:/Windows/Fonts/arial.ttf" << std::endl;
    } else {
        std::cout << "[Space Defender] Notice: TrueType font not loaded. Visual HUD shapes will be active." << std::endl;
    }

    if (m_fontLoaded) {
        // --- In-Game HUD Elements ---
        m_scoreText.setFont(m_font);
        m_scoreText.setCharacterSize(14);
        m_scoreText.setFillColor(sf::Color(0, 230, 255));
        m_scoreText.setPosition(25.0f, 20.0f);

        m_waveText.setFont(m_font);
        m_waveText.setCharacterSize(14);
        m_waveText.setFillColor(sf::Color(255, 210, 60));
        m_waveText.setPosition(430.0f, 20.0f);

        m_healthText.setFont(m_font);
        m_healthText.setCharacterSize(12);
        m_healthText.setFillColor(sf::Color(220, 240, 255));
        m_healthText.setPosition(730.0f, 15.0f);

        // --- Username Entry Screen Elements ---
        m_usernameTitleText.setFont(m_font);
        m_usernameTitleText.setCharacterSize(42);
        m_usernameTitleText.setFillColor(sf::Color(0, 240, 255));
        m_usernameTitleText.setString("SPACE DEFENDER");
        sf::FloatRect utBounds = m_usernameTitleText.getLocalBounds();
        m_usernameTitleText.setOrigin(utBounds.left + utBounds.width / 2.0f, utBounds.top + utBounds.height / 2.0f);
        m_usernameTitleText.setPosition(WINDOW_WIDTH / 2.0f, 130.0f);

        m_usernameSubtitleText.setFont(m_font);
        m_usernameSubtitleText.setCharacterSize(12);
        m_usernameSubtitleText.setFillColor(sf::Color(255, 210, 60));
        m_usernameSubtitleText.setString("GALACTIC DEFENSE INITIATIVE");
        sf::FloatRect usBounds = m_usernameSubtitleText.getLocalBounds();
        m_usernameSubtitleText.setOrigin(usBounds.left + usBounds.width / 2.0f, usBounds.top + usBounds.height / 2.0f);
        m_usernameSubtitleText.setPosition(WINDOW_WIDTH / 2.0f, 180.0f);

        m_usernamePromptText.setFont(m_font);
        m_usernamePromptText.setCharacterSize(15);
        m_usernamePromptText.setFillColor(sf::Color(200, 230, 255));
        m_usernamePromptText.setString("ENTER YOUR USERNAME:");
        sf::FloatRect upBounds = m_usernamePromptText.getLocalBounds();
        m_usernamePromptText.setOrigin(upBounds.left + upBounds.width / 2.0f, upBounds.top + upBounds.height / 2.0f);
        m_usernamePromptText.setPosition(WINDOW_WIDTH / 2.0f, 260.0f);

        m_usernameInputText.setFont(m_font);
        m_usernameInputText.setCharacterSize(20);
        m_usernameInputText.setFillColor(sf::Color(0, 255, 200));

        m_usernameValidationText.setFont(m_font);
        m_usernameValidationText.setCharacterSize(12);
        m_usernameValidationText.setFillColor(sf::Color(255, 80, 80));

        m_usernameLimitText.setFont(m_font);
        m_usernameLimitText.setCharacterSize(11);
        m_usernameLimitText.setFillColor(sf::Color(130, 165, 205));
        m_usernameLimitText.setString("MAXIMUM 15 CHARACTERS  |  [A-Z, 0-9, _]");
        sf::FloatRect ulBounds = m_usernameLimitText.getLocalBounds();
        m_usernameLimitText.setOrigin(ulBounds.left + ulBounds.width / 2.0f, ulBounds.top + ulBounds.height / 2.0f);
        m_usernameLimitText.setPosition(WINDOW_WIDTH / 2.0f, 445.0f);

        m_usernameInstructionText.setFont(m_font);
        m_usernameInstructionText.setCharacterSize(13);
        m_usernameInstructionText.setFillColor(sf::Color(0, 255, 200));
        m_usernameInstructionText.setString("PRESS [ENTER] TO CONTINUE");
        sf::FloatRect uiBounds = m_usernameInstructionText.getLocalBounds();
        m_usernameInstructionText.setOrigin(uiBounds.left + uiBounds.width / 2.0f, uiBounds.top + uiBounds.height / 2.0f);
        m_usernameInstructionText.setPosition(WINDOW_WIDTH / 2.0f, 550.0f);

        // --- Game Over Screen Elements ---
        m_gameOverText.setFont(m_font);
        m_gameOverText.setCharacterSize(40);
        m_gameOverText.setFillColor(sf::Color(255, 45, 65));
        m_gameOverText.setString("GAME OVER");
        sf::FloatRect goBounds = m_gameOverText.getLocalBounds();
        m_gameOverText.setOrigin(goBounds.left + goBounds.width / 2.0f, goBounds.top + goBounds.height / 2.0f);
        m_gameOverText.setPosition(WINDOW_WIDTH / 2.0f, 170.0f);

        m_finalScoreText.setFont(m_font);
        m_finalScoreText.setCharacterSize(20);
        m_finalScoreText.setFillColor(sf::Color(255, 220, 100));

        m_gameOverUserText.setFont(m_font);
        m_gameOverUserText.setCharacterSize(16);
        m_gameOverUserText.setFillColor(sf::Color(0, 230, 255));

        m_gameOverSavedText.setFont(m_font);
        m_gameOverSavedText.setCharacterSize(13);
        m_gameOverSavedText.setFillColor(sf::Color(80, 255, 180));
        m_gameOverSavedText.setString("SCORE SAVED TO LEADERBOARD");

        m_instructionText.setFont(m_font);
        m_instructionText.setCharacterSize(12);
        m_instructionText.setFillColor(sf::Color(170, 195, 220));
        m_instructionText.setString("[L] LEADERBOARD  |  [R] RESTART MISSION  |  [ESC] MAIN MENU");
        sf::FloatRect insBounds = m_instructionText.getLocalBounds();
        m_instructionText.setOrigin(insBounds.left + insBounds.width / 2.0f, insBounds.top + insBounds.height / 2.0f);
        m_instructionText.setPosition(WINDOW_WIDTH / 2.0f, 460.0f);

        // --- Leaderboard Screen (uses separate readable monospaced font) ---
        m_leaderboardFontLoaded = false;
        if (m_leaderboardFont.loadFromFile("assets/fonts/ConsoleFont.ttf")) {
            m_leaderboardFontLoaded = true;
        } else if (m_leaderboardFont.loadFromFile("C:/Windows/Fonts/cour.ttf")) {
            m_leaderboardFontLoaded = true;
        }
        // Determine which font to use for leaderboard elements
        sf::Font& lbFont = m_leaderboardFontLoaded ? m_leaderboardFont : m_font;

        // Header: "--- GALACTIC LEADERBOARD ---"
        m_leaderboardHeaderText.setFont(lbFont);
        m_leaderboardHeaderText.setCharacterSize(36);
        m_leaderboardHeaderText.setFillColor(sf::Color(0, 230, 255));
        m_leaderboardHeaderText.setString("--- GALACTIC LEADERBOARD ---");
        sf::FloatRect lbHBounds = m_leaderboardHeaderText.getLocalBounds();
        m_leaderboardHeaderText.setOrigin(lbHBounds.left + lbHBounds.width / 2.0f, lbHBounds.top + lbHBounds.height / 2.0f);
        m_leaderboardHeaderText.setPosition(WINDOW_WIDTH / 2.0f, 48.0f);

        // Algo text – kept for internal state; NOT drawn on screen
        m_leaderboardAlgoText.setFont(lbFont);
        m_leaderboardAlgoText.setCharacterSize(20);
        m_leaderboardAlgoText.setFillColor(sf::Color(255, 210, 60));

        // Metrics line
        m_leaderboardMetricsText.setFont(lbFont);
        m_leaderboardMetricsText.setCharacterSize(20);
        m_leaderboardMetricsText.setFillColor(sf::Color(180, 220, 255));

        // Column heading row: RANK / USERNAME / SCORE
        m_leaderboardColumnText.setFont(lbFont);
        m_leaderboardColumnText.setCharacterSize(22);
        m_leaderboardColumnText.setFillColor(sf::Color(255, 210, 60));
        m_leaderboardColumnText.setString("RANK   USERNAME                SCORE");
        m_leaderboardColumnText.setPosition(80.0f, 148.0f);

        // Data rows
        m_leaderboardRowsText.setFont(lbFont);
        m_leaderboardRowsText.setCharacterSize(21);
        m_leaderboardRowsText.setFillColor(sf::Color(240, 245, 255));
        m_leaderboardRowsText.setPosition(80.0f, 178.0f);

        // Bottom controls
        m_leaderboardControlsText.setFont(lbFont);
        m_leaderboardControlsText.setCharacterSize(18);
        m_leaderboardControlsText.setFillColor(sf::Color(150, 180, 210));
        m_leaderboardControlsText.setString("[1] Bubble Sort  [2] Selection Sort  [3] Insertion Sort\n[B] Benchmark  [R] Restart  [ESC] Back");
        sf::FloatRect lbCBounds = m_leaderboardControlsText.getLocalBounds();
        m_leaderboardControlsText.setOrigin(lbCBounds.left + lbCBounds.width / 2.0f, lbCBounds.top + lbCBounds.height / 2.0f);
        m_leaderboardControlsText.setPosition(WINDOW_WIDTH / 2.0f, 648.0f);

        // --- Main Menu Elements ---
        m_menuTitleText.setFont(m_font);
        m_menuTitleText.setCharacterSize(42);
        m_menuTitleText.setFillColor(sf::Color(0, 240, 255));
        m_menuTitleText.setString("SPACE DEFENDER");
        sf::FloatRect mtBounds = m_menuTitleText.getLocalBounds();
        m_menuTitleText.setOrigin(mtBounds.left + mtBounds.width / 2.0f, mtBounds.top + mtBounds.height / 2.0f);
        m_menuTitleText.setPosition(WINDOW_WIDTH / 2.0f, 150.0f);

        m_menuSubtitleText.setFont(m_font);
        m_menuSubtitleText.setCharacterSize(12);
        m_menuSubtitleText.setFillColor(sf::Color(255, 210, 60));
        m_menuSubtitleText.setString("GALACTIC DEFENSE INITIATIVE");
        sf::FloatRect msBounds = m_menuSubtitleText.getLocalBounds();
        m_menuSubtitleText.setOrigin(msBounds.left + msBounds.width / 2.0f, msBounds.top + msBounds.height / 2.0f);
        m_menuSubtitleText.setPosition(WINDOW_WIDTH / 2.0f, 210.0f);

        const std::string menuOptions[4] = {
            "START MISSION",
            "LEADERBOARD",
            "HOW TO PLAY / CONTROLS",
            "EXIT"
        };
        for (int i = 0; i < 4; ++i) {
            m_menuOptionsText[i].setFont(m_font);
            m_menuOptionsText[i].setCharacterSize(16);
            m_menuOptionsText[i].setString(menuOptions[i]);
        }

        // --- Pause Screen Elements ---
        m_pauseTitleText.setFont(m_font);
        m_pauseTitleText.setCharacterSize(38);
        m_pauseTitleText.setFillColor(sf::Color(255, 210, 60));
        m_pauseTitleText.setString("MISSION PAUSED");
        sf::FloatRect ptBounds = m_pauseTitleText.getLocalBounds();
        m_pauseTitleText.setOrigin(ptBounds.left + ptBounds.width / 2.0f, ptBounds.top + ptBounds.height / 2.0f);
        m_pauseTitleText.setPosition(WINDOW_WIDTH / 2.0f, 220.0f);

        const std::string pauseOptions[3] = {
            "RESUME FLIGHT",
            "RESTART MISSION",
            "ABORT TO MAIN MENU"
        };
        for (int i = 0; i < 3; ++i) {
            m_pauseOptionsText[i].setFont(m_font);
            m_pauseOptionsText[i].setCharacterSize(16);
            m_pauseOptionsText[i].setString(pauseOptions[i]);
        }

        // --- Controls Screen Elements ---
        m_controlsTitleText.setFont(m_font);
        m_controlsTitleText.setCharacterSize(26);
        m_controlsTitleText.setFillColor(sf::Color(0, 240, 255));
        m_controlsTitleText.setString("--- FLIGHT CONTROLS & SETTINGS ---");
        sf::FloatRect ctBounds = m_controlsTitleText.getLocalBounds();
        m_controlsTitleText.setOrigin(ctBounds.left + ctBounds.width / 2.0f, ctBounds.top + ctBounds.height / 2.0f);
        m_controlsTitleText.setPosition(WINDOW_WIDTH / 2.0f, 100.0f);

        m_controlsBodyText.setFont(m_font);
        m_controlsBodyText.setCharacterSize(13);
        m_controlsBodyText.setFillColor(sf::Color(230, 240, 255));
        m_controlsBodyText.setPosition(140.0f, 180.0f);

        m_controlsPromptText.setFont(m_font);
        m_controlsPromptText.setCharacterSize(12);
        m_controlsPromptText.setFillColor(sf::Color(170, 200, 230));
        m_controlsPromptText.setString("PRESS [ESC] OR [ENTER] TO RETURN TO MENU");
        sf::FloatRect cpBounds = m_controlsPromptText.getLocalBounds();
        m_controlsPromptText.setOrigin(cpBounds.left + cpBounds.width / 2.0f, cpBounds.top + cpBounds.height / 2.0f);
        m_controlsPromptText.setPosition(WINDOW_WIDTH / 2.0f, 620.0f);
    }

    // Health Bar Frame & Fill
    m_healthBarBg.setSize(sf::Vector2f(220.0f, 16.0f));
    m_healthBarBg.setPosition(730.0f, 34.0f);
    m_healthBarBg.setFillColor(sf::Color(15, 20, 35));
    m_healthBarBg.setOutlineColor(sf::Color(0, 220, 255));
    m_healthBarBg.setOutlineThickness(1.5f);

    m_healthBarFill.setSize(sf::Vector2f(220.0f, 16.0f));
    m_healthBarFill.setPosition(730.0f, 34.0f);
    m_healthBarFill.setFillColor(sf::Color(0, 220, 255));

    // Username Input Box Frame
    m_usernameInputBox.setSize(sf::Vector2f(440.0f, 50.0f));
    m_usernameInputBox.setOrigin(220.0f, 25.0f);
    m_usernameInputBox.setPosition(WINDOW_WIDTH / 2.0f, 335.0f);
    m_usernameInputBox.setFillColor(sf::Color(12, 16, 32, 220));
    m_usernameInputBox.setOutlineColor(sf::Color(0, 220, 255));
    m_usernameInputBox.setOutlineThickness(2.0f);

    // Full Viewport Shroud Overlay
    m_gameOverOverlay.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    m_gameOverOverlay.setFillColor(sf::Color(8, 10, 22, 230));
}

void Game::initWaves() {
    while (!m_enemyWaves.empty()) {
        m_enemyWaves.pop();
    }

    m_enemyWaves.push(5);   // Wave 1
    m_enemyWaves.push(10);  // Wave 2
    m_enemyWaves.push(15);  // Wave 3
    m_enemyWaves.push(20);  // Wave 4
    m_enemyWaves.push(25);  // Wave 5

    std::cout << "[Space Defender] Wave queue initialized with 5 waves." << std::endl;
}

void Game::run() {
    sf::Clock clock;

    while (m_window.isOpen()) {
        sf::Time deltaTime = clock.restart();

        processEvents();
        update(deltaTime);
        render();

        m_frameCount++;

        if (m_testMode && m_frameCount >= 60) {
            std::cout << "[Space Defender] Test mode completed successfully ("
                      << m_frameCount << " frames rendered)." << std::endl;
            m_window.close();
            break;
        }
    }

    std::cout << "[Space Defender] Game loop ended. Window closed cleanly." << std::endl;
}

bool Game::isRunning() const {
    return m_window.isOpen();
}

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            std::cout << "[Space Defender] Window close event received." << std::endl;
            m_window.close();
        } else if (event.type == sf::Event::TextEntered) {
            if (m_gameState == GameState::UsernameEntry) {
                handleTextInput(event.text.unicode);
            }
        } else if (event.type == sf::Event::KeyPressed) {
            // --- State: USERNAME ENTRY ---
            if (m_gameState == GameState::UsernameEntry) {
                if (event.key.code == sf::Keyboard::Escape) {
                    m_window.close();
                }
            }
            // --- State: MAIN MENU ---
            else if (m_gameState == GameState::MainMenu) {
                if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                    m_menuSelectedIndex = (m_menuSelectedIndex - 1 + 4) % 4;
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                    m_menuSelectedIndex = (m_menuSelectedIndex + 1) % 4;
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    m_audio.playMenuSelect();
                    if (m_menuSelectedIndex == 0) {
                        resetGame();
                        m_gameState = GameState::Playing;
                    } else if (m_menuSelectedIndex == 1) {
                        m_previousState = GameState::MainMenu;
                        m_gameState = GameState::Leaderboard;
                    } else if (m_menuSelectedIndex == 2) {
                        m_gameState = GameState::Controls;
                    } else if (m_menuSelectedIndex == 3) {
                        m_window.close();
                    }
                } else if (event.key.code == sf::Keyboard::Escape) {
                    m_window.close();
                }
            }
            // --- State: PLAYING ---
            else if (m_gameState == GameState::Playing) {
                if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape) {
                    m_gameState = GameState::Paused;
                    m_pauseSelectedIndex = 0;
                    m_audio.playMenuSelect();
                }
            }
            // --- State: PAUSED ---
            else if (m_gameState == GameState::Paused) {
                if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                    m_pauseSelectedIndex = (m_pauseSelectedIndex - 1 + 3) % 3;
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                    m_pauseSelectedIndex = (m_pauseSelectedIndex + 1) % 3;
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    m_audio.playMenuSelect();
                    if (m_pauseSelectedIndex == 0) {
                        m_gameState = GameState::Playing; // Resume
                    } else if (m_pauseSelectedIndex == 1) {
                        resetGame();
                        m_gameState = GameState::Playing; // Restart
                    } else if (m_pauseSelectedIndex == 2) {
                        m_gameState = GameState::MainMenu; // Abort
                    }
                } else if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape) {
                    m_gameState = GameState::Playing; // Resume on toggle
                }
            }
            // --- State: CONTROLS / SETTINGS ---
            else if (m_gameState == GameState::Controls) {
                if (event.key.code == sf::Keyboard::M) {
                    m_audio.toggleMute();
                } else if (event.key.code == sf::Keyboard::Equal || event.key.code == sf::Keyboard::Add) {
                    m_audio.setVolume(m_audio.getVolume() + 10.0f);
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Dash || event.key.code == sf::Keyboard::Subtract) {
                    m_audio.setVolume(m_audio.getVolume() - 10.0f);
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Return) {
                    m_gameState = GameState::MainMenu;
                    m_audio.playMenuSelect();
                }
            }
            // --- State: GAME OVER ---
            else if (m_gameState == GameState::GameOver) {
                if (event.key.code == sf::Keyboard::L) {
                    m_previousState = GameState::GameOver;
                    m_gameState = GameState::Leaderboard;
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::R) {
                    resetGame();
                    m_gameState = GameState::Playing;
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::Escape ||
                           event.key.code == sf::Keyboard::Return ||
                           event.key.code == sf::Keyboard::Space) {
                    m_gameState = GameState::MainMenu;
                    m_audio.playMenuSelect();
                }
            }
            // --- State: LEADERBOARD ---
            else if (m_gameState == GameState::Leaderboard) {
                if (event.key.code == sf::Keyboard::Num1) {
                    m_leaderboard.bubbleSort(&m_lastMetrics);
                    m_audio.playMenuSelect();
                    updateWindowTitle();
                } else if (event.key.code == sf::Keyboard::Num2) {
                    m_leaderboard.selectionSort(&m_lastMetrics);
                    m_audio.playMenuSelect();
                    updateWindowTitle();
                } else if (event.key.code == sf::Keyboard::Num3) {
                    m_leaderboard.insertionSort(&m_lastMetrics);
                    m_audio.playMenuSelect();
                    updateWindowTitle();
                } else if (event.key.code == sf::Keyboard::B) {
                    runSortingBenchmark();
                    m_audio.playMenuSelect();
                } else if (event.key.code == sf::Keyboard::R) {
                    resetGame();
                    m_gameState = GameState::Playing;
                } else if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::BackSpace || event.key.code == sf::Keyboard::Tab) {
                    m_gameState = m_previousState;
                    m_audio.playMenuSelect();
                }
            }
        }
    }
}

void Game::handleTextInput(sf::Uint32 unicode) {
    if (m_gameState != GameState::UsernameEntry) {
        return;
    }

    if (unicode == 8) { // Backspace
        if (!m_playerNameInput.empty()) {
            m_playerNameInput.pop_back();
            m_usernameValidationError.clear();
        }
    } else if (unicode == 13 || unicode == 10) { // Enter
        if (m_playerNameInput.empty()) {
            m_usernameValidationError = "USERNAME CANNOT BE EMPTY";
            m_audio.playPlayerDamage();
        } else {
            m_usernameValidationError.clear();
            m_gameState = GameState::MainMenu;
            m_audio.playMenuSelect();
            std::cout << "[Space Defender] Username confirmed: " << m_playerNameInput << std::endl;
        }
    } else if (unicode < 128) {
        char c = static_cast<char>(unicode);
        if ((std::isalnum(static_cast<unsigned char>(c)) || c == '_') &&
            m_playerNameInput.length() < Leaderboard::MAX_NAME_LENGTH) {
            m_playerNameInput += c;
            m_usernameValidationError.clear();
        }
    }
}

void Game::resetGame() {
    m_player = Player(static_cast<float>(WINDOW_WIDTH) / 2.0f, static_cast<float>(WINDOW_HEIGHT) - 80.0f);
    m_bullets.clear();
    m_enemies.clear();
    m_vfx.clearParticles();
    m_score = 0;
    m_currentWave = 0;
    m_enemiesRemainingToSpawn = 0;
    m_totalEnemiesDefeated = 0;
    m_isGameOver = false;
    m_scoreSubmitted = false;
    // NOTE: m_playerNameInput is preserved throughout session and across restarts!

    initWaves();
    updateHUD();
    updateWindowTitle();
    std::cout << "[Space Defender] Game reset. Mission restarted with username: "
              << m_playerNameInput << std::endl;
}

void Game::update(sf::Time deltaTime) {
    // 1. Starfield background is always animated across all states
    m_vfx.update(deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 2. Only advance combat physics and waves when actively PLAYING
    if (m_gameState != GameState::Playing) {
        return;
    }

    m_player.handleInput();
    m_player.update(deltaTime, m_window.getSize());

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        fireBullet();
    }

    if (m_testMode && (m_frameCount == 5 || m_frameCount == 25)) {
        fireBullet();
    }

    updateBullets(deltaTime);
    updateWaves(deltaTime);
    updateEnemies(deltaTime);
    updateCollisions();
    updateHUD();
}

void Game::fireBullet() {
    if (m_gameState != GameState::Playing) return;

    if (m_shootClock.getElapsedTime() >= m_shootCooldown) {
        m_bullets.emplace_back(m_player.getMuzzlePosition());
        m_vfx.spawnImpact(m_player.getMuzzlePosition());
        m_audio.playShoot();
        m_shootClock.restart();
    }
}

void Game::updateBullets(sf::Time deltaTime) {
    for (auto& bullet : m_bullets) {
        bullet.update(deltaTime);
    }
}

void Game::updateWaves(sf::Time deltaTime) {
    (void)deltaTime;

    if (m_gameState != GameState::Playing) return;

    if (m_enemiesRemainingToSpawn == 0 && m_enemies.empty()) {
        if (!m_enemyWaves.empty()) {
            int nextWaveCount = m_enemyWaves.front();
            m_enemyWaves.pop();

            m_currentWave++;
            m_enemiesRemainingToSpawn = nextWaveCount;

            // Progressive difficulty: speed increases by 20 px/s, spawn time tightens
            float spawnSecs = std::max(0.7f, 1.4f - (m_currentWave - 1) * 0.12f);
            m_enemySpawnInterval = sf::seconds(spawnSecs);

            std::cout << "[Space Defender] >>> WAVE " << m_currentWave
                      << " INCOMING! Total enemies: " << nextWaveCount
                      << " (Queued waves left: " << m_enemyWaves.size() << ") <<<" << std::endl;

            updateWindowTitle();
        } else {
            static bool victoryLogged = false;
            if (!victoryLogged) {
                std::cout << "[Space Defender] All planned enemy waves cleared! Sector secured." << std::endl;
                victoryLogged = true;
            }
        }
    }

    if (m_enemiesRemainingToSpawn > 0 && m_enemySpawnClock.getElapsedTime() >= m_enemySpawnInterval) {
        spawnEnemy();
        m_enemySpawnClock.restart();
    }
}

void Game::spawnEnemy() {
    if (m_gameState != GameState::Playing) return;

    float spawnX = m_xDist(m_rng);
    float spawnY = -Enemy::ENEMY_HEIGHT / 2.0f;
    float waveSpeed = Enemy::BASE_SPEED + (m_currentWave - 1) * 20.0f;

    m_enemies.emplace_back(spawnX, spawnY, waveSpeed);
    m_enemiesRemainingToSpawn--;
    updateWindowTitle();
}

void Game::updateEnemies(sf::Time deltaTime) {
    for (auto& enemy : m_enemies) {
        enemy.update(deltaTime);
    }

    for (auto& enemy : m_enemies) {
        if (!enemy.isDestroyed() && enemy.isOffScreen(static_cast<float>(WINDOW_HEIGHT))) {
            enemy.destroy();
            m_player.takeDamage(Enemy::ESCAPE_DAMAGE);
            m_vfx.triggerDamageFlash();
            m_audio.playEnemyEscape();

            std::cout << "[Space Defender] Enemy escaped! -10 HP. Player HP: "
                      << m_player.getHealth() << std::endl;

            if (m_player.getHealth() <= 0) {
                triggerGameOver();
            }
        }
    }
}

void Game::updateCollisions() {
    if (m_gameState != GameState::Playing) return;

    // 1. Bullet vs Enemy Collisions
    for (auto& bullet : m_bullets) {
        if (bullet.isDestroyed()) continue;

        for (auto& enemy : m_enemies) {
            if (enemy.isDestroyed()) continue;

            if (bullet.getBounds().intersects(enemy.getBounds())) {
                bullet.destroy();
                enemy.destroy();
                m_score += Enemy::SCORE_VALUE;
                m_totalEnemiesDefeated++;

                // Visual explosion and audio
                m_vfx.spawnExplosion(enemy.getPosition(), sf::Color(255, 60, 80));
                m_audio.playExplosion();

                std::cout << "[Space Defender] Direct Hit! +100 Points. Total Score: "
                          << m_score << std::endl;
                break;
            }
        }
    }

    // 2. Player vs Enemy Collisions
    if (!m_isGameOver) {
        for (auto& enemy : m_enemies) {
            if (enemy.isDestroyed()) continue;

            if (m_player.getBounds().intersects(enemy.getBounds())) {
                enemy.destroy();
                m_player.takeDamage(Enemy::COLLISION_DAMAGE);

                m_vfx.spawnExplosion(enemy.getPosition(), sf::Color(255, 140, 40));
                m_vfx.triggerDamageFlash();
                m_audio.playPlayerDamage();

                std::cout << "[Space Defender] Hull Collision! -20 HP. Player HP: "
                          << m_player.getHealth() << std::endl;

                if (m_player.getHealth() <= 0) {
                    triggerGameOver();
                    break;
                }
            }
        }
    }

    // 3. Safe Vector Cleanup (Erase-Remove)
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const Bullet& b) {
                return b.isOffScreen(0.0f) || b.isDestroyed();
            }),
        m_bullets.end()
    );

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& e) {
                return e.isOffScreen(static_cast<float>(WINDOW_HEIGHT)) || e.isDestroyed();
            }),
        m_enemies.end()
    );
}

void Game::triggerGameOver() {
    if (m_isGameOver) return;

    m_isGameOver = true;
    m_gameState = GameState::GameOver;

    // Automatically submit score using stored username
    if (!m_scoreSubmitted && !m_playerNameInput.empty()) {
        if (m_leaderboard.addEntry(m_playerNameInput, m_score)) {
            m_leaderboard.saveToFile();
            m_scoreSubmitted = true;
            std::cout << "[Leaderboard] Record saved automatically: " << m_playerNameInput
                      << " with score " << m_score << std::endl;
        }
    }

    m_audio.playGameOver();

    std::cout << "[Space Defender] =========================================" << std::endl;
    std::cout << "[Space Defender]               GAME OVER                  " << std::endl;
    std::cout << "[Space Defender] Pilot Username: " << m_playerNameInput << std::endl;
    std::cout << "[Space Defender] Final Score: " << m_score << std::endl;
    std::cout << "[Space Defender] Waves Reached: " << m_currentWave << std::endl;
    std::cout << "[Space Defender] Enemies Eliminated: " << m_totalEnemiesDefeated << std::endl;
    std::cout << "[Space Defender] =========================================" << std::endl;

    updateHUD();
    updateWindowTitle();
}

void Game::runSortingBenchmark() {
    AlgorithmMetrics bMetrics = m_leaderboard.benchmarkSort(SortAlgorithm::BubbleSort);
    AlgorithmMetrics sMetrics = m_leaderboard.benchmarkSort(SortAlgorithm::SelectionSort);
    AlgorithmMetrics iMetrics = m_leaderboard.benchmarkSort(SortAlgorithm::InsertionSort);

    std::cout << "\n================ ALGORITHM BENCHMARK COMPARISON ================" << std::endl;
    std::cout << "Data Set Size: " << m_leaderboard.size() << " records" << std::endl;
    std::cout << std::left << std::setw(18) << "Algorithm"
              << std::setw(15) << "Comparisons"
              << std::setw(16) << "Swaps/Shifts"
              << std::setw(16) << "Time (us)" << std::endl;
    std::cout << "----------------------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(18) << "Bubble Sort"
              << std::setw(15) << bMetrics.comparisons
              << std::setw(16) << bMetrics.swapsOrMoves
              << std::setw(16) << bMetrics.durationMicroseconds << std::endl;
    std::cout << std::left << std::setw(18) << "Selection Sort"
              << std::setw(15) << sMetrics.comparisons
              << std::setw(16) << sMetrics.swapsOrMoves
              << std::setw(16) << sMetrics.durationMicroseconds << std::endl;
    std::cout << std::left << std::setw(18) << "Insertion Sort"
              << std::setw(15) << iMetrics.comparisons
              << std::setw(16) << iMetrics.swapsOrMoves
              << std::setw(16) << iMetrics.durationMicroseconds << std::endl;
    std::cout << "================================================================\n" << std::endl;

    m_leaderboard.sort(m_leaderboard.getCurrentAlgorithm(), &m_lastMetrics);
}

void Game::updateHUD() {
    if (m_fontLoaded) {
        m_scoreText.setString("SCORE: " + std::to_string(m_score));
        m_waveText.setString("WAVE: " + std::to_string(m_currentWave));
        m_healthText.setString("HULL: " + std::to_string(m_player.getHealth()) + "/100");

        m_finalScoreText.setString("FINAL SCORE: " + std::to_string(m_score));
        sf::FloatRect fsBounds = m_finalScoreText.getLocalBounds();
        m_finalScoreText.setOrigin(fsBounds.left + fsBounds.width / 2.0f, fsBounds.top + fsBounds.height / 2.0f);
        m_finalScoreText.setPosition(WINDOW_WIDTH / 2.0f, 250.0f);

        m_gameOverUserText.setString("PILOT USERNAME: " + m_playerNameInput);
        sf::FloatRect guBounds = m_gameOverUserText.getLocalBounds();
        m_gameOverUserText.setOrigin(guBounds.left + guBounds.width / 2.0f, guBounds.top + guBounds.height / 2.0f);
        m_gameOverUserText.setPosition(WINDOW_WIDTH / 2.0f, 310.0f);

        m_gameOverSavedText.setString(m_scoreSubmitted ? "SCORE SAVED TO LEADERBOARD" : "SCORE RECORDED");
        sf::FloatRect gsBounds = m_gameOverSavedText.getLocalBounds();
        m_gameOverSavedText.setOrigin(gsBounds.left + gsBounds.width / 2.0f, gsBounds.top + gsBounds.height / 2.0f);
        m_gameOverSavedText.setPosition(WINDOW_WIDTH / 2.0f, 360.0f);
    }

    float healthRatio = static_cast<float>(m_player.getHealth()) / 100.0f;
    healthRatio = std::clamp(healthRatio, 0.0f, 1.0f);
    m_healthBarFill.setSize(sf::Vector2f(220.0f * healthRatio, 16.0f));

    if (m_player.getHealth() > 50) {
        m_healthBarFill.setFillColor(sf::Color(0, 220, 255));
    } else if (m_player.getHealth() > 20) {
        m_healthBarFill.setFillColor(sf::Color(255, 160, 40));
    } else {
        m_healthBarFill.setFillColor(sf::Color(255, 45, 65));
    }
}

void Game::updateWindowTitle() {
    std::string title = std::string(WINDOW_TITLE) +
        " | Score: " + std::to_string(m_score) +
        " | Wave: " + std::to_string(m_currentWave) +
        " | HP: " + std::to_string(m_player.getHealth());

    if (m_gameState == GameState::UsernameEntry) {
        title = "Space Defender | Enter Username";
    } else if (m_gameState == GameState::MainMenu) {
        title = "Space Defender | Main Menu";
    } else if (m_gameState == GameState::Paused) {
        title += " [PAUSED]";
    } else if (m_gameState == GameState::Controls) {
        title = "Space Defender | Controls & Settings";
    } else if (m_gameState == GameState::GameOver) {
        title += " [GAME OVER]";
    } else if (m_gameState == GameState::Leaderboard) {
        title += " [LEADERBOARD: " + Leaderboard::getAlgorithmName(m_leaderboard.getCurrentAlgorithm()) + "]";
    }

    m_window.setTitle(title);
}

void Game::renderHUD() {
    m_window.draw(m_healthBarBg);
    m_window.draw(m_healthBarFill);

    if (m_fontLoaded) {
        m_window.draw(m_scoreText);
        m_window.draw(m_waveText);
        m_window.draw(m_healthText);
    }
}

void Game::renderMainMenu() {
    m_window.draw(m_gameOverOverlay);

    if (!m_fontLoaded) return;

    m_window.draw(m_menuTitleText);
    m_window.draw(m_menuSubtitleText);

    const float startY = 300.0f;
    const float spacing = 60.0f;

    for (int i = 0; i < 4; ++i) {
        if (i == m_menuSelectedIndex) {
            m_menuOptionsText[i].setFillColor(sf::Color(0, 255, 200));
            m_menuOptionsText[i].setString("> " + std::string(
                i == 0 ? "START MISSION" :
                i == 1 ? "LEADERBOARD" :
                i == 2 ? "HOW TO PLAY / CONTROLS" : "EXIT"
            ) + " <");
        } else {
            m_menuOptionsText[i].setFillColor(sf::Color(160, 185, 215));
            m_menuOptionsText[i].setString(
                i == 0 ? "START MISSION" :
                i == 1 ? "LEADERBOARD" :
                i == 2 ? "HOW TO PLAY / CONTROLS" : "EXIT"
            );
        }

        sf::FloatRect bounds = m_menuOptionsText[i].getLocalBounds();
        m_menuOptionsText[i].setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        m_menuOptionsText[i].setPosition(WINDOW_WIDTH / 2.0f, startY + (i * spacing));
        m_window.draw(m_menuOptionsText[i]);
    }

    // Footer prompt
    sf::Text footerText;
    footerText.setFont(m_font);
    footerText.setCharacterSize(11);
    footerText.setFillColor(sf::Color(130, 160, 190));
    footerText.setString("NAVIGATE: [W/S] OR [ARROWS]  |  SELECT: [ENTER]  |  EXIT: [ESC]");
    sf::FloatRect fBounds = footerText.getLocalBounds();
    footerText.setOrigin(fBounds.left + fBounds.width / 2.0f, fBounds.top + fBounds.height / 2.0f);
    footerText.setPosition(WINDOW_WIDTH / 2.0f, 620.0f);
    m_window.draw(footerText);
}

void Game::renderPaused() {
    m_window.draw(m_gameOverOverlay);

    if (!m_fontLoaded) return;

    m_window.draw(m_pauseTitleText);

    const float startY = 320.0f;
    const float spacing = 55.0f;

    for (int i = 0; i < 3; ++i) {
        if (i == m_pauseSelectedIndex) {
            m_pauseOptionsText[i].setFillColor(sf::Color(0, 255, 200));
            m_pauseOptionsText[i].setString("> " + std::string(
                i == 0 ? "RESUME FLIGHT" :
                i == 1 ? "RESTART MISSION" : "ABORT TO MAIN MENU"
            ) + " <");
        } else {
            m_pauseOptionsText[i].setFillColor(sf::Color(160, 185, 215));
            m_pauseOptionsText[i].setString(
                i == 0 ? "RESUME FLIGHT" :
                i == 1 ? "RESTART MISSION" : "ABORT TO MAIN MENU"
            );
        }

        sf::FloatRect bounds = m_pauseOptionsText[i].getLocalBounds();
        m_pauseOptionsText[i].setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        m_pauseOptionsText[i].setPosition(WINDOW_WIDTH / 2.0f, startY + (i * spacing));
        m_window.draw(m_pauseOptionsText[i]);
    }
}

void Game::renderControls() {
    m_window.draw(m_gameOverOverlay);

    if (!m_fontLoaded) return;

    m_window.draw(m_controlsTitleText);

    std::ostringstream oss;
    oss << "MANEUVER SHIP:     [W] [A] [S] [D]  or  [ARROW KEYS]\n\n"
        << "FIRE CANNONS:      [SPACEBAR]\n\n"
        << "PAUSE / RESUME:    [P]  or  [ESCAPE]\n\n"
        << "AUDIO SOUND FX:    [M] TOGGLE MUTE  ("
        << (m_audio.isMuted() ? "MUTED" : "ACTIVE") << ")\n\n"
        << "MASTER VOLUME:     [+] INCREASE  [-] DECREASE  ("
        << static_cast<int>(m_audio.getVolume()) << "%)\n\n"
        << "COMBAT RULES:      DEFEND PERIMETER. BREACHES COST 10 HP.\n"
        << "                   SHIP-TO-SHIP COLLISIONS COST 20 HP.\n";

    m_controlsBodyText.setString(oss.str());
    m_window.draw(m_controlsBodyText);
    m_window.draw(m_controlsPromptText);
}

void Game::renderUsernameEntry() {
    m_window.draw(m_gameOverOverlay);

    if (!m_fontLoaded) return;

    m_window.draw(m_usernameTitleText);
    m_window.draw(m_usernameSubtitleText);
    m_window.draw(m_usernamePromptText);

    // Draw input box
    m_window.draw(m_usernameInputBox);

    // Dynamic text with blinking cursor
    std::string displayStr = m_playerNameInput;
    if ((m_cursorClock.getElapsedTime().asMilliseconds() / 500) % 2 == 0) {
        displayStr += "_";
    }

    m_usernameInputText.setString(displayStr);
    sf::FloatRect textBounds = m_usernameInputText.getLocalBounds();
    m_usernameInputText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                 textBounds.top + textBounds.height / 2.0f);
    m_usernameInputText.setPosition(WINDOW_WIDTH / 2.0f, 335.0f);
    m_window.draw(m_usernameInputText);

    // Display validation error if present
    if (!m_usernameValidationError.empty()) {
        m_usernameValidationText.setString(m_usernameValidationError);
        sf::FloatRect vBounds = m_usernameValidationText.getLocalBounds();
        m_usernameValidationText.setOrigin(vBounds.left + vBounds.width / 2.0f,
                                          vBounds.top + vBounds.height / 2.0f);
        m_usernameValidationText.setPosition(WINDOW_WIDTH / 2.0f, 395.0f);
        m_window.draw(m_usernameValidationText);
    }

    m_window.draw(m_usernameLimitText);
    m_window.draw(m_usernameInstructionText);
}

void Game::renderGameOver() {
    m_window.draw(m_gameOverOverlay);

    if (!m_fontLoaded) return;

    m_window.draw(m_gameOverText);
    m_window.draw(m_finalScoreText);
    m_window.draw(m_gameOverUserText);
    m_window.draw(m_gameOverSavedText);
    m_window.draw(m_instructionText);
}

void Game::renderLeaderboard() {
    m_window.draw(m_gameOverOverlay);

    if (!m_fontLoaded) return;

    // Header
    m_window.draw(m_leaderboardHeaderText);

    // Build algo info string (kept for state; NOT drawn — label was removed by design)
    std::string algoInfo = "SORTING ALGORITHM: " +
        Leaderboard::getAlgorithmName(m_leaderboard.getCurrentAlgorithm()) + " (DESCENDING)";
    m_leaderboardAlgoText.setString(algoInfo);

    // Metrics line
    std::ostringstream oss;
    oss << "Comparisons: " << m_lastMetrics.comparisons
        << "  |  Swaps/Moves: " << m_lastMetrics.swapsOrMoves
        << "  |  Time: " << std::fixed << std::setprecision(1)
        << m_lastMetrics.durationMicroseconds << " us";
    m_leaderboardMetricsText.setString(oss.str());
    sf::FloatRect mBounds = m_leaderboardMetricsText.getLocalBounds();
    m_leaderboardMetricsText.setOrigin(mBounds.left + mBounds.width / 2.0f, mBounds.top + mBounds.height / 2.0f);
    m_leaderboardMetricsText.setPosition(WINDOW_WIDTH / 2.0f, 100.0f);
    m_window.draw(m_leaderboardMetricsText);

    // Column headings
    m_window.draw(m_leaderboardColumnText);

    // Separator line drawn via a thin rectangle
    sf::RectangleShape sep(sf::Vector2f(WINDOW_WIDTH - 160.0f, 1.0f));
    sep.setFillColor(sf::Color(80, 120, 160));
    sep.setPosition(80.0f, 174.0f);
    m_window.draw(sep);

    // Data rows
    std::ostringstream tableStream;
    const auto& entries = m_leaderboard.getEntries();
    size_t count = std::min(entries.size(), Leaderboard::MAX_DISPLAY_ENTRIES);

    if (entries.empty()) {
        tableStream << "\n   NO SCORES RECORDED YET\n";
    } else {
        for (size_t i = 0; i < count; ++i) {
            std::string rankStr = std::to_string(i + 1) + ".";
            tableStream << std::left
                        << std::setw(7)  << rankStr
                        << std::setw(24) << entries[i].name
                        << std::right << std::setw(8) << entries[i].score << "\n";
        }
    }

    m_leaderboardRowsText.setString(tableStream.str());
    m_leaderboardRowsText.setPosition(80.0f, 178.0f);
    m_window.draw(m_leaderboardRowsText);

    // Bottom controls
    m_window.draw(m_leaderboardControlsText);
}

void Game::render() {
    m_window.clear(sf::Color(10, 12, 28));

    // 1. Render Parallax Starfield & Dynamic Particles
    m_vfx.render(m_window);

    // 2. Render Gameplay entities when active, paused, or game over
    if (m_gameState == GameState::Playing ||
        m_gameState == GameState::Paused ||
        m_gameState == GameState::GameOver) {

        for (const auto& bullet : m_bullets) {
            bullet.render(m_window);
        }
        for (const auto& enemy : m_enemies) {
            enemy.render(m_window);
        }
        if (!m_isGameOver || m_player.isAlive()) {
            m_player.render(m_window);
        }
        renderHUD();
    }

    // 3. State-Specific Overlays
    switch (m_gameState) {
        case GameState::UsernameEntry:
            renderUsernameEntry();
            break;
        case GameState::MainMenu:
            renderMainMenu();
            break;
        case GameState::Paused:
            renderPaused();
            break;
        case GameState::Controls:
            renderControls();
            break;
        case GameState::GameOver:
            renderGameOver();
            break;
        case GameState::Leaderboard:
            renderLeaderboard();
            break;
        case GameState::Playing:
            break;
    }

    m_window.display();
}
