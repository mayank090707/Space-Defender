#pragma once

#include <SFML/Graphics.hpp>

/**
 * @brief Represents the player-controlled spaceship in Space Defender.
 * 
 * Encapsulates player state, input handling, physics-based movement,
 * screen boundary collisions, and multi-shape rendering.
 */
class Player {
public:
    static constexpr float DEFAULT_SPEED = 380.0f; // Pixels per second
    static constexpr int DEFAULT_HEALTH = 100;
    static constexpr float SHIP_WIDTH = 46.0f;
    static constexpr float SHIP_HEIGHT = 48.0f;

    /**
     * @brief Constructs the player ship at a specified screen position.
     * @param startX Initial X coordinate.
     * @param startY Initial Y coordinate.
     */
    explicit Player(float startX = 500.0f, float startY = 620.0f);
    ~Player() = default;

    /**
     * @brief Polls real-time keyboard state to determine movement intent.
     */
    void handleInput();

    /**
     * @brief Updates player position with delta time and enforces window boundaries.
     * @param deltaTime Time elapsed since previous frame.
     * @param windowSize Current dimensions of the game window.
     */
    void update(sf::Time deltaTime, const sf::Vector2u& windowSize);

    /**
     * @brief Draws the player ship and components to the render target.
     * @param target The SFML render target (e.g. sf::RenderWindow).
     */
    void render(sf::RenderTarget& target) const;

    // --- State Getters & Setters ---
    [[nodiscard]] sf::Vector2f getPosition() const;
    [[nodiscard]] sf::Vector2f getMuzzlePosition() const;
    void setPosition(const sf::Vector2f& position);
    void setPosition(float x, float y);

    [[nodiscard]] int getHealth() const;
    void setHealth(int health);
    void takeDamage(int amount);
    [[nodiscard]] bool isAlive() const;

    [[nodiscard]] float getSpeed() const;
    void setSpeed(float speed);

    [[nodiscard]] sf::FloatRect getBounds() const;

private:
    void initVisuals();
    void updateVisualPositions();
    void clampToBounds(const sf::Vector2u& windowSize);

    // State
    sf::Vector2f m_position;
    sf::Vector2f m_direction;
    float m_speed;
    int m_health;

    // Visual shapes composing the futuristic spaceship
    sf::ConvexShape m_hull;
    sf::ConvexShape m_cockpit;
    sf::ConvexShape m_leftWingAccent;
    sf::ConvexShape m_rightWingAccent;
    sf::RectangleShape m_thruster;
};
