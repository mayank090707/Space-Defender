#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

/**
 * @brief Represents an individual dynamic particle (spark, explosion debris).
 */
struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    float size;
};

/**
 * @brief Represents a star in the parallax starfield background.
 */
struct Star {
    sf::Vector2f position;
    float speed;
    float size;
    sf::Color color;
};

/**
 * @brief Manages procedural visual effects: multi-layer starfield, explosion particles,
 *        impact sparks, and damage flash feedback.
 */
class VisualEffects {
public:
    static constexpr size_t STAR_COUNT = 90;

    VisualEffects();
    ~VisualEffects() = default;

    void init(unsigned int windowWidth, unsigned int windowHeight);
    void update(sf::Time deltaTime, unsigned int windowWidth, unsigned int windowHeight);
    void render(sf::RenderTarget& target) const;

    // Trigger visual events
    void spawnExplosion(const sf::Vector2f& position, const sf::Color& primaryColor = sf::Color(255, 60, 40));
    void spawnImpact(const sf::Vector2f& position);
    void triggerDamageFlash();

    void clearParticles();

private:
    std::vector<Star> m_stars;
    std::vector<Particle> m_particles;
    std::mt19937 m_rng;

    // Damage flash screen effect
    float m_damageFlashAlpha;
    sf::RectangleShape m_flashOverlay;
};
