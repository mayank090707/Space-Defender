#include "Bullet.hpp"

Bullet::Bullet(float startX, float startY, float speed)
    : m_position(startX, startY)
    , m_speed(speed)
    , m_isDestroyed(false)
{
    initVisuals();
}

Bullet::Bullet(const sf::Vector2f& startPosition, float speed)
    : Bullet(startPosition.x, startPosition.y, speed)
{
}

void Bullet::initVisuals() {
    // Outer electric cyan glow bolt
    m_shape.setSize(sf::Vector2f(BULLET_WIDTH, BULLET_HEIGHT));
    m_shape.setOrigin(BULLET_WIDTH / 2.0f, BULLET_HEIGHT / 2.0f);
    m_shape.setFillColor(sf::Color(0, 225, 255));
    m_shape.setOutlineColor(sf::Color(100, 240, 255, 180));
    m_shape.setOutlineThickness(1.0f);
    m_shape.setPosition(m_position);

    // Inner bright white-cyan energy core
    m_core.setSize(sf::Vector2f(BULLET_WIDTH * 0.5f, BULLET_HEIGHT * 0.75f));
    m_core.setOrigin((BULLET_WIDTH * 0.5f) / 2.0f, (BULLET_HEIGHT * 0.75f) / 2.0f);
    m_core.setFillColor(sf::Color(240, 255, 255));
    m_core.setPosition(m_position);
}

void Bullet::update(sf::Time deltaTime) {
    // Translate upward (decreasing Y coordinate)
    m_position.y -= m_speed * deltaTime.asSeconds();

    m_shape.setPosition(m_position);
    m_core.setPosition(m_position);
}

void Bullet::render(sf::RenderTarget& target) const {
    target.draw(m_shape);
    target.draw(m_core);
}

bool Bullet::isOffScreen(float topBoundary) const {
    // If the bottom edge of the bullet has passed above the top boundary
    return (m_position.y + (BULLET_HEIGHT / 2.0f)) < topBoundary;
}

sf::Vector2f Bullet::getPosition() const {
    return m_position;
}

float Bullet::getSpeed() const {
    return m_speed;
}

sf::FloatRect Bullet::getBounds() const {
    return sf::FloatRect(
        m_position.x - (BULLET_WIDTH / 2.0f),
        m_position.y - (BULLET_HEIGHT / 2.0f),
        BULLET_WIDTH,
        BULLET_HEIGHT
    );
}

void Bullet::destroy() {
    m_isDestroyed = true;
}

bool Bullet::isDestroyed() const {
    return m_isDestroyed;
}
