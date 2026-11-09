#pragma once

#include <SFML/Graphics.hpp>

/**
 * @brief Represents an alien raider enemy descending towards the player.
 * 
 * Encapsulates enemy movement, health, bounds, off-screen escape detection,
 * and procedural multi-shape rendering.
 */
class Enemy {
public:
    static constexpr float BASE_SPEED = 160.0f; // Default pixels per second downward
    static constexpr int DEFAULT_HEALTH = 1;
    static constexpr float ENEMY_WIDTH = 42.0f;
    static constexpr float ENEMY_HEIGHT = 40.0f;
    static constexpr int ESCAPE_DAMAGE = 10;     // Damage dealt to player if escaped
    static constexpr int COLLISION_DAMAGE = 20;  // Damage dealt to player on ship-to-ship collision
    static constexpr int SCORE_VALUE = 100;      // Score awarded when destroyed by bullet

    /**
     * @brief Constructs an enemy at a specified coordinate.
     * @param startX Initial X coordinate.
     * @param startY Initial Y coordinate.
     * @param speed Downward velocity in pixels per second.
     * @param health Enemy hit points.
     */
    explicit Enemy(float startX, float startY, float speed = BASE_SPEED, int health = DEFAULT_HEALTH);
    explicit Enemy(const sf::Vector2f& startPosition, float speed = BASE_SPEED, int health = DEFAULT_HEALTH);
    ~Enemy() = default;

    /**
     * @brief Translates the enemy downward based on delta time.
     * @param deltaTime Elapsed time since previous frame.
     */
    void update(sf::Time deltaTime);

    /**
     * @brief Draws the enemy spaceship to the render target.
     * @param target Render target to draw onto.
     */
    void render(sf::RenderTarget& target) const;

    /**
     * @brief Checks if the enemy has passed entirely beyond the bottom screen boundary.
     * @param bottomBoundary Screen height threshold.
     * @return True if the enemy has escaped off-screen.
     */
    [[nodiscard]] bool isOffScreen(float bottomBoundary) const;

    // --- Accessors & Mutators ---
    [[nodiscard]] sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f& position);
    void setPosition(float x, float y);

    [[nodiscard]] int getHealth() const;
    void setHealth(int health);
    void takeDamage(int amount);
    void destroy();
    [[nodiscard]] bool isDestroyed() const;

    [[nodiscard]] float getSpeed() const;
    void setSpeed(float speed);

    [[nodiscard]] sf::FloatRect getBounds() const;

private:
    void initVisuals();
    void updateVisualPositions();

    sf::Vector2f m_position;
    float m_speed;
    int m_health;

    // Procedural SFML shapes for menacing alien raider
    sf::ConvexShape m_hull;
    sf::ConvexShape m_cockpit;
    sf::ConvexShape m_leftWing;
    sf::ConvexShape m_rightWing;
    sf::RectangleShape m_thruster;
};
