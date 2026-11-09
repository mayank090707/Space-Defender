#include "VisualEffects.hpp"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

VisualEffects::VisualEffects()
    : m_rng(std::random_device{}())
    , m_damageFlashAlpha(0.0f)
{
}

void VisualEffects::init(unsigned int windowWidth, unsigned int windowHeight) {
    m_stars.clear();
    m_stars.reserve(STAR_COUNT);

    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(windowWidth));
    std::uniform_real_distribution<float> distY(0.0f, static_cast<float>(windowHeight));
    std::uniform_int_distribution<int> layerDist(0, 2);

    for (size_t i = 0; i < STAR_COUNT; ++i) {
        int layer = layerDist(m_rng);
        Star star;
        star.position = {distX(m_rng), distY(m_rng)};

        if (layer == 0) {
            // Far layer: subtle, dim, slow
            star.speed = 35.0f;
            star.size = 1.0f;
            star.color = sf::Color(100, 130, 170, 140);
        } else if (layer == 1) {
            // Mid layer: cyan tint, medium
            star.speed = 80.0f;
            star.size = 1.5f;
            star.color = sf::Color(120, 190, 235, 200);
        } else {
            // Near layer: bright, prominent, fast
            star.speed = 150.0f;
            star.size = 2.0f;
            star.color = sf::Color(220, 245, 255, 255);
        }
        m_stars.push_back(star);
    }

    m_flashOverlay.setSize(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    m_flashOverlay.setFillColor(sf::Color(255, 0, 0, 0));
}

void VisualEffects::update(sf::Time deltaTime, unsigned int windowWidth, unsigned int windowHeight) {
    float dt = deltaTime.asSeconds();

    // 1. Update Starfield
    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(windowWidth));
    for (auto& star : m_stars) {
        star.position.y += star.speed * dt;
        if (star.position.y > static_cast<float>(windowHeight)) {
            star.position.y = 0.0f;
            star.position.x = distX(m_rng);
        }
    }

    // 2. Update Dynamic Particles
    for (auto& p : m_particles) {
        p.position += p.velocity * dt;
        p.lifetime -= dt;
    }

    // Safe Erase-Remove for expired particles
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p) {
                return p.lifetime <= 0.0f;
            }),
        m_particles.end()
    );

    // 3. Update Damage Flash Screen Overlay
    if (m_damageFlashAlpha > 0.0f) {
        m_damageFlashAlpha = std::max(0.0f, m_damageFlashAlpha - (dt * 450.0f));
        m_flashOverlay.setFillColor(sf::Color(255, 30, 30, static_cast<sf::Uint8>(m_damageFlashAlpha)));
    }
}

void VisualEffects::render(sf::RenderTarget& target) const {
    // 1. Draw Starfield
    for (const auto& star : m_stars) {
        sf::RectangleShape starShape(sf::Vector2f(star.size, star.size));
        starShape.setPosition(star.position);
        starShape.setFillColor(star.color);
        target.draw(starShape);
    }

    // 2. Draw Dynamic Particles
    for (const auto& p : m_particles) {
        float alphaRatio = std::clamp(p.lifetime / p.maxLifetime, 0.0f, 1.0f);
        sf::Color renderColor = p.color;
        renderColor.a = static_cast<sf::Uint8>(p.color.a * alphaRatio);

        sf::RectangleShape particleShape(sf::Vector2f(p.size, p.size));
        particleShape.setOrigin(p.size / 2.0f, p.size / 2.0f);
        particleShape.setPosition(p.position);
        particleShape.setFillColor(renderColor);
        target.draw(particleShape);
    }

    // 3. Draw Damage Flash
    if (m_damageFlashAlpha > 0.0f) {
        target.draw(m_flashOverlay);
    }
}

void VisualEffects::spawnExplosion(const sf::Vector2f& position, const sf::Color& primaryColor) {
    std::uniform_real_distribution<float> angleDist(0.0f, static_cast<float>(2.0 * M_PI));
    std::uniform_real_distribution<float> speedDist(90.0f, 260.0f);
    std::uniform_real_distribution<float> lifeDist(0.35f, 0.65f);
    std::uniform_real_distribution<float> sizeDist(2.0f, 4.0f);
    std::uniform_int_distribution<int> colorVariation(0, 2);

    const size_t particleCount = 18;
    for (size_t i = 0; i < particleCount; ++i) {
        float angle = angleDist(m_rng);
        float speed = speedDist(m_rng);
        float life = lifeDist(m_rng);

        sf::Color col = primaryColor;
        int var = colorVariation(m_rng);
        if (var == 1) {
            col = sf::Color(255, 160, 40); // Fiery amber
        } else if (var == 2) {
            col = sf::Color(255, 230, 90); // White-yellow heat
        }

        Particle p;
        p.position = position;
        p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.color = col;
        p.lifetime = life;
        p.maxLifetime = life;
        p.size = sizeDist(m_rng);
        m_particles.push_back(p);
    }
}

void VisualEffects::spawnImpact(const sf::Vector2f& position) {
    std::uniform_real_distribution<float> angleDist(0.0f, static_cast<float>(2.0 * M_PI));
    std::uniform_real_distribution<float> speedDist(50.0f, 130.0f);
    std::uniform_real_distribution<float> lifeDist(0.15f, 0.30f);

    const size_t particleCount = 6;
    for (size_t i = 0; i < particleCount; ++i) {
        float angle = angleDist(m_rng);
        float speed = speedDist(m_rng);
        float life = lifeDist(m_rng);

        Particle p;
        p.position = position;
        p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.color = sf::Color(0, 240, 255); // Cyan plasma spark
        p.lifetime = life;
        p.maxLifetime = life;
        p.size = 2.0f;
        m_particles.push_back(p);
    }
}

void VisualEffects::triggerDamageFlash() {
    m_damageFlashAlpha = 110.0f; // Max semi-transparent flash
}

void VisualEffects::clearParticles() {
    m_particles.clear();
    m_damageFlashAlpha = 0.0f;
}
