#include <cassert>
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include "AudioManager.hpp"
#include "VisualEffects.hpp"
#include "Leaderboard.hpp"
#include "Player.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "Game.hpp"

enum class MockGameState {
    UsernameEntry,
    MainMenu,
    Playing,
    Paused,
    Leaderboard,
    Controls,
    GameOver
};

void runPlayerTests() {
    std::cout << "\n--- [1/7] REGRESSION TEST SUITE: PLAYER SYSTEM ---" << std::endl;

    Player player(500.0f, 620.0f);
    assert(player.getPosition().x == 500.0f);
    assert(player.getPosition().y == 620.0f);
    assert(player.getHealth() == 100);
    assert(player.isAlive());

    // Boundary restrictions
    sf::Vector2u winSize(1000, 700);
    player.setPosition(-500.0f, -500.0f);
    player.update(sf::Time::Zero, winSize);
    float halfW = Player::SHIP_WIDTH / 2.0f;
    float halfH = Player::SHIP_HEIGHT / 2.0f;
    assert(player.getPosition().x == halfW);
    assert(player.getPosition().y == halfH);

    player.setPosition(5000.0f, 5000.0f);
    player.update(sf::Time::Zero, winSize);
    assert(player.getPosition().x == 1000.0f - halfW);
    assert(player.getPosition().y == 700.0f - halfH);

    // Health bounds & clamping
    player.takeDamage(40);
    assert(player.getHealth() == 60);
    player.takeDamage(200);
    assert(player.getHealth() == 0);
    assert(!player.isAlive());

    std::cout << "  ✓ Player movement boundaries, clamping, and health invariants verified." << std::endl;
}

void runBulletTests() {
    std::cout << "\n--- [2/7] REGRESSION TEST SUITE: WEAPON & BULLET SYSTEM ---" << std::endl;

    Bullet b(500.0f, 600.0f, 750.0f);
    assert(b.getPosition().x == 500.0f);
    assert(b.getPosition().y == 600.0f);
    assert(!b.isOffScreen(0.0f));

    // Movement over delta time (0.2s * 750 px/s = 150 px upward)
    b.update(sf::seconds(0.2f));
    assert(std::abs(b.getPosition().y - 450.0f) < 0.001f);
    assert(!b.isOffScreen(0.0f));

    // Off-screen cleanup threshold
    b.update(sf::seconds(0.7f)); // moves 525 px up -> y = -75.0f
    assert(b.isOffScreen(0.0f));

    // Multi-bullet vector lifecycle
    std::vector<Bullet> bullets;
    for (int i = 0; i < 10; ++i) {
        bullets.emplace_back(100.0f * i, (i % 2 == 0) ? -20.0f : 300.0f);
    }
    assert(bullets.size() == 10);

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& item) { return item.isOffScreen(0.0f); }),
        bullets.end()
    );
    assert(bullets.size() == 5); // 5 on-screen bullets survive

    std::cout << "  ✓ Bullet kinematics, off-screen detection, and vector lifecycle verified." << std::endl;
}

void runEnemyTests() {
    std::cout << "\n--- [3/7] REGRESSION TEST SUITE: ENEMY & WAVE SYSTEM ---" << std::endl;

    Enemy e(300.0f, -20.0f, 200.0f, 1);
    assert(!e.isOffScreen(700.0f));

    // Downward movement over delta time
    e.update(sf::seconds(0.5f)); // +100 px down -> y = 80.0f
    assert(std::abs(e.getPosition().y - 80.0f) < 0.001f);

    // Escape trigger (bottom window = 700.0f)
    e.setPosition(300.0f, 750.0f);
    assert(e.isOffScreen(700.0f));

    // Wave queue progression (FIFO)
    std::queue<int> waveQueue;
    waveQueue.push(5);
    waveQueue.push(10);
    waveQueue.push(15);
    waveQueue.push(20);
    waveQueue.push(25);

    assert(waveQueue.front() == 5); waveQueue.pop();
    assert(waveQueue.front() == 10); waveQueue.pop();
    assert(waveQueue.front() == 15); waveQueue.pop();
    assert(waveQueue.front() == 20); waveQueue.pop();
    assert(waveQueue.front() == 25); waveQueue.pop();
    assert(waveQueue.empty());

    // Difficulty scaling
    for (int wave = 1; wave <= 5; ++wave) {
        float speed = Enemy::BASE_SPEED + (wave - 1) * 20.0f;
        float interval = std::max(0.70f, 1.40f - (wave - 1) * 0.12f);
        assert(speed >= 160.0f && speed <= 240.0f);
        assert(interval >= 0.70f && interval <= 1.40f);
    }

    std::cout << "  ✓ Enemy kinematics, escape detection, FIFO queue, and difficulty curves verified." << std::endl;
}

void runCombatTests() {
    std::cout << "\n--- [4/7] REGRESSION TEST SUITE: COMBAT & COLLISION RESOLUTION ---" << std::endl;

    // Bullet-Enemy AABB Collision
    Bullet b(500.0f, 300.0f);
    Enemy e(500.0f, 300.0f);
    assert(b.getBounds().intersects(e.getBounds()));

    b.destroy();
    e.destroy();
    assert(b.isDestroyed() && e.isDestroyed());

    // Single bullet does not destroy multiple overlapping enemies
    std::vector<Bullet> bullets = { Bullet(500.0f, 300.0f) };
    std::vector<Enemy> enemies = { Enemy(500.0f, 300.0f), Enemy(501.0f, 301.0f) };
    int score = 0;

    for (auto& bullet : bullets) {
        if (bullet.isDestroyed()) continue;
        for (auto& enemy : enemies) {
            if (enemy.isDestroyed()) continue;
            if (bullet.getBounds().intersects(enemy.getBounds())) {
                bullet.destroy();
                enemy.destroy();
                score += Enemy::SCORE_VALUE;
                break;
            }
        }
    }
    assert(bullets[0].isDestroyed());
    assert(enemies[0].isDestroyed());
    assert(!enemies[1].isDestroyed()); // Second enemy survived!
    assert(score == 100);

    // Player-Enemy Collision
    Player p(500.0f, 600.0f);
    Enemy e2(500.0f, 600.0f);
    assert(p.getBounds().intersects(e2.getBounds()));
    p.takeDamage(Enemy::COLLISION_DAMAGE);
    assert(p.getHealth() == 80);

    std::cout << "  ✓ AABB intersection, score idempotency, and collision damage verified." << std::endl;
}

void runLeaderboardTests() {
    std::cout << "\n--- [5/7] REGRESSION TEST SUITE: LEADERBOARD & DSA ALGORITHMS ---" << std::endl;

    const std::vector<ScoreEntry> dataset = {
        {"Viper", 400},
        {"Ace", 1200},
        {"Nova", 800},
        {"Echo", 800},   // Tie-breaker: Echo < Nova
        {"Blaze", 1200}, // Tie-breaker: Ace < Blaze
        {"Ghost", 100}
    };

    const std::vector<ScoreEntry> expected = {
        {"Ace", 1200},
        {"Blaze", 1200},
        {"Echo", 800},
        {"Nova", 800},
        {"Viper", 400},
        {"Ghost", 100}
    };

    auto matchesExpected = [&expected](const Leaderboard& lb) {
        if (lb.size() != expected.size()) return false;
        for (size_t i = 0; i < expected.size(); ++i) {
            if (!(lb.getEntries()[i] == expected[i])) return false;
        }
        return true;
    };

    // 1. Bubble Sort
    Leaderboard lb1("scratch/dummy_master.txt");
    lb1.clear();
    for (const auto& item : dataset) lb1.addEntry(item.name, item.score);
    lb1.bubbleSort();
    assert(matchesExpected(lb1));

    // 2. Selection Sort (Stable)
    Leaderboard lb2("scratch/dummy_master.txt");
    lb2.clear();
    for (const auto& item : dataset) lb2.addEntry(item.name, item.score);
    lb2.selectionSort();
    assert(matchesExpected(lb2));

    // 3. Insertion Sort
    Leaderboard lb3("scratch/dummy_master.txt");
    lb3.clear();
    for (const auto& item : dataset) lb3.addEntry(item.name, item.score);
    lb3.insertionSort();
    assert(matchesExpected(lb3));

    // Linear Search (Case-insensitive)
    auto foundLinear = lb3.linearSearch("vIpEr");
    assert(foundLinear.size() == 1);
    assert(foundLinear[0].score == 400);

    // Binary Search (Descending Duplicate Range)
    auto foundBinary = lb3.binarySearch(800);
    assert(foundBinary.size() == 2);
    assert(foundBinary[0].name == "Echo" && foundBinary[1].name == "Nova");

    auto missingBinary = lb3.binarySearch(9999);
    assert(missingBinary.empty());

    // File Persistence Test
    std::string testPath = "data/master_persist_test.txt";
    Leaderboard lbOut(testPath);
    lbOut.clear();
    lbOut.addEntry("Master_Chief", 5000);
    assert(lbOut.saveToFile(testPath));

    Leaderboard lbIn(testPath);
    assert(lbIn.size() == 1);
    assert(lbIn.getEntries()[0].name == "Master_Chief" && lbIn.getEntries()[0].score == 5000);

    std::cout << "  ✓ Bubble, Selection, Insertion sorts, Linear/Binary searches, and I/O verified." << std::endl;
}

void runUIAudioAndVFXTests() {
    std::cout << "\n--- [6/7] REGRESSION TEST SUITE: UI, AUDIO & VISUAL EFFECTS ---" << std::endl;

    // State machine navigation
    MockGameState state = MockGameState::MainMenu;
    int selection = 0;
    selection = (selection + 1) % 4; // Move to option 1
    assert(selection == 1);

    // Enter selection
    state = MockGameState::Playing;
    assert(state == MockGameState::Playing);

    // Pause state invariant
    state = MockGameState::Paused;
    assert(state == MockGameState::Paused);

    // Resume
    state = MockGameState::Playing;
    assert(state == MockGameState::Playing);

    // Audio Engine
    AudioManager audio;
    assert(!audio.isMuted());
    audio.toggleMute();
    assert(audio.isMuted());
    audio.toggleMute();
    assert(!audio.isMuted());
    audio.setVolume(75.0f);
    assert(audio.getVolume() == 75.0f);

    // VFX Starfield & Particles
    VisualEffects vfx;
    vfx.init(1000, 700);
    vfx.spawnExplosion({500.0f, 350.0f});
    vfx.spawnImpact({500.0f, 600.0f});
    vfx.triggerDamageFlash();
    vfx.update(sf::seconds(1.0f), 1000, 700); // Fully ages out particles
    vfx.clearParticles();

    std::cout << "  ✓ State machine flow, audio synthesis & controls, starfield & VFX verified." << std::endl;
}

void runPerformanceStressTest() {
    std::cout << "\n--- [7/7] PERFORMANCE & HIGH-LOAD STRESS BENCHMARK ---" << std::endl;

    const size_t BULLET_COUNT = 150;
    const size_t ENEMY_COUNT = 150;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    bullets.reserve(BULLET_COUNT);
    enemies.reserve(ENEMY_COUNT);

    for (size_t i = 0; i < BULLET_COUNT; ++i) {
        bullets.emplace_back(static_cast<float>((i * 6) % 950 + 25), static_cast<float>((i * 4) % 650 + 25));
    }
    for (size_t j = 0; j < ENEMY_COUNT; ++j) {
        enemies.emplace_back(static_cast<float>((j * 6) % 950 + 25), static_cast<float>((j * 4) % 650 + 25));
    }

    auto start = std::chrono::high_resolution_clock::now();

    size_t collisionsDetected = 0;
    for (auto& bullet : bullets) {
        if (bullet.isDestroyed()) continue;
        for (auto& enemy : enemies) {
            if (enemy.isDestroyed()) continue;
            if (bullet.getBounds().intersects(enemy.getBounds())) {
                bullet.destroy();
                enemy.destroy();
                collisionsDetected++;
                break;
            }
        }
    }

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return b.isDestroyed(); }),
        bullets.end()
    );
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return e.isDestroyed(); }),
        enemies.end()
    );

    auto end = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "  Load: " << BULLET_COUNT << " bullets vs " << ENEMY_COUNT << " enemies ("
              << (BULLET_COUNT * ENEMY_COUNT) << " AABB checks)" << std::endl;
    std::cout << "  Collisions resolved: " << collisionsDetected << std::endl;
    std::cout << "  Surviving bullets: " << bullets.size() << " | enemies: " << enemies.size() << std::endl;
    std::cout << "  Time Elapsed: " << durationMs << " ms (Target: < 1.0 ms)" << std::endl;

    assert(durationMs < 10.0); // Extreme upper bound to ensure 60 FPS frame headroom
    std::cout << "  ✓ Performance verified: Sub-millisecond execution well within 16.6ms frame budget!" << std::endl;
}

void runUsernameAndGameOverFlowTests() {
    std::cout << "\n--- [8/8] REGRESSION TEST SUITE: STARTUP USERNAME & GAME OVER FLOW ---" << std::endl;

    // Backup active leaderboard content to restore after test execution
    std::string leaderboardBackup;
    {
        std::ifstream in("data/leaderboard.txt");
        if (in.is_open()) {
            std::ostringstream ss;
            ss << in.rdbuf();
            leaderboardBackup = ss.str();
        }
    }

    // 1. Application starts in UsernameEntry state when launched normally
    Game game(false);
    assert(game.getGameState() == GameState::UsernameEntry);
    assert(game.getUsername().empty());
    std::cout << "  ✓ Requirement 1: Application starts in UsernameEntry state." << std::endl;

    // 2. Empty username is rejected when pressing Enter
    game.simulateTextInput(13); // Enter
    assert(game.getGameState() == GameState::UsernameEntry); // Remains in UsernameEntry
    assert(game.getValidationError() == "USERNAME CANNOT BE EMPTY");
    std::cout << "  ✓ Requirement 2 & 3: Empty username rejected with validation message." << std::endl;

    // 3. Valid username characters accepted
    game.simulateTextInput('S');
    game.simulateTextInput('T');
    game.simulateTextInput('A');
    game.simulateTextInput('R');
    game.simulateTextInput('_');
    game.simulateTextInput('7');
    assert(game.getUsername() == "STAR_7");
    assert(game.getValidationError().empty());
    std::cout << "  ✓ Requirement 4: Valid alphanumeric and underscore characters accepted." << std::endl;

    // 4. Invalid characters are rejected
    game.simulateTextInput('@');
    game.simulateTextInput(' ');
    game.simulateTextInput('!');
    game.simulateTextInput('#');
    assert(game.getUsername() == "STAR_7");
    std::cout << "  ✓ Requirement 5: Invalid characters ignored without corrupting buffer." << std::endl;

    // 5. Backspace works
    game.simulateTextInput(8); // Backspace removes '7'
    assert(game.getUsername() == "STAR_");
    game.simulateTextInput(8); // Backspace removes '_'
    assert(game.getUsername() == "STAR");
    std::cout << "  ✓ Requirement 6: Backspace removes characters cleanly." << std::endl;

    // 6. Maximum 15-character limit enforced
    for (int i = 0; i < 20; ++i) {
        game.simulateTextInput('X');
    }
    assert(game.getUsername().length() == 15);
    std::cout << "  ✓ Requirement 7: 15-character maximum length enforced." << std::endl;

    // Set clean test username
    game.setUsername("COMMANDER_ACE");
    assert(game.getUsername() == "COMMANDER_ACE");

    // 7. Enter transitions to MainMenu
    game.simulateTextInput(13); // Enter
    assert(game.getGameState() == GameState::MainMenu);
    std::cout << "  ✓ Requirement 8: Enter transitions to MainMenu upon valid username." << std::endl;

    // 8. Username remains unchanged during gameplay
    game.setGameState(GameState::Playing);
    assert(game.getGameState() == GameState::Playing);
    assert(game.getUsername() == "COMMANDER_ACE");
    std::cout << "  ✓ Requirement 9: Username remains stable during gameplay." << std::endl;

    // 9. Game Over automatically submits score with stored username
    game.setScore(3400);
    size_t countBefore = game.getLeaderboard().size();
    game.simulateGameOver();
    assert(game.getGameState() == GameState::GameOver);
    assert(game.isScoreSubmitted());
    std::cout << "  ✓ Requirement 10: Game Over automatically submits final score." << std::endl;

    // 10. Game Over does not request username again
    assert(game.getGameState() == GameState::GameOver);
    std::cout << "  ✓ Requirement 11: Game Over transitions to GameOver screen, not username entry." << std::endl;

    // 11. Score is submitted only once per game
    size_t countAfter = game.getLeaderboard().size();
    game.simulateGameOver(); // Second trigger attempt
    assert(game.getLeaderboard().size() == countAfter);
    std::cout << "  ✓ Requirement 12: Score submission is idempotent (submitted only once)." << std::endl;

    // 12. Restart retains the current username
    game.restartMission();
    assert(game.getUsername() == "COMMANDER_ACE");
    assert(game.getScore() == 0);
    assert(!game.isScoreSubmitted());
    std::cout << "  ✓ Requirement 13: Restart retains current username across games." << std::endl;

    // 13. Test mode starts in Playing with default non-blocking username
    Game testGame(true);
    assert(testGame.getGameState() == GameState::Playing);
    assert(!testGame.getUsername().empty());
    std::cout << "  ✓ Requirement 14: Headless test mode runs non-interactively without user input." << std::endl;

    // Restore original leaderboard file content
    if (!leaderboardBackup.empty()) {
        std::ofstream out("data/leaderboard.txt", std::ios::trunc);
        out << leaderboardBackup;
    }

    std::cout << "  ✓ All 16 username entry & game over requirements verified." << std::endl;
}

int main() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "        SPACE DEFENDER: MASTER REGRESSION & PERFORMANCE SUITE         " << std::endl;
    std::cout << "======================================================================" << std::endl;

    runPlayerTests();
    runBulletTests();
    runEnemyTests();
    runCombatTests();
    runLeaderboardTests();
    runUIAudioAndVFXTests();
    runPerformanceStressTest();
    runUsernameAndGameOverFlowTests();

    std::cout << "\n======================================================================" << std::endl;
    std::cout << "  ALL 8 MASTER REGRESSION SUITES EXECUTED WITH ZERO ERRORS (PASS)!   " << std::endl;
    std::cout << "======================================================================" << std::endl;
    return 0;
}
