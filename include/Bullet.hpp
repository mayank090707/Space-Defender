#pragma once

#include <SFML/Graphics.hpp>

/**
 * @brief Represents a laser projectile fired by the player.
 * 
 * Encapsulates bullet position, speed, rendering, off-screen lifetime detection,
 * and collision bounding box.
 */
class Bullet {
public:
    static constexpr float DEFAULT_SPEED = 750.0f; // Pixels per second
    static constexpr float BULLET_WIDTH = 4.0f;
    static constexpr float BULLET_HEIGHT = 18.0f;

    /**
     * @brief Constructs a bullet at a specified spawn point.
     * @param startX Initial X coordinate (typically ship muzzle X).
     * @param startY Initial Y coordinate (typically ship muzzle Y).
     * @param speed Velocity in pixels per second directed upward.
     */
    explicit Bullet(float startX, float startY, float speed = DEFAULT_SPEED);
    explicit Bullet(const sf::Vector2f& startPosition, float speed = DEFAULT_SPEED);
    ~Bullet() = default;

    /**
     * @brief Updates bullet position based on delta time.
     * @param deltaTime Elapsed time since previous frame.
     */
    void update(sf::Time deltaTime);

    /**
     * @brief Renders the laser bolt to the render target.
     * @param target Render target to draw onto.
     */
    void render(sf::RenderTarget& target) const;

    /**
     * @brief Checks if the bullet has completely moved past the top boundary.
     * @param topBoundary Upper Y threshold (defaults to 0.0f).
     * @return True if off-screen and eligible for memory reclamation.
     */
    [[nodiscard]] bool isOffScreen(float topBoundary = 0.0f) const;

    // --- Accessors ---
    [[nodiscard]] sf::Vector2f getPosition() const;
    [[nodiscard]] float getSpeed() const;
    [[nodiscard]] sf::FloatRect getBounds() const;

    void destroy();
    [[nodiscard]] bool isDestroyed() const;

private:
    void initVisuals();

    sf::Vector2f m_position;
    float m_speed;
    bool m_isDestroyed;
    sf::RectangleShape m_shape;
    sf::RectangleShape m_core;
};
