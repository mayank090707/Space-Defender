#include "Enemy.hpp"
#include <algorithm>

Enemy::Enemy(float startX, float startY, float speed, int health)
    : m_position(startX, startY)
    , m_speed(speed)
    , m_health(health)
{
    initVisuals();
    updateVisualPositions();
}

Enemy::Enemy(const sf::Vector2f& startPosition, float speed, int health)
    : Enemy(startPosition.x, startPosition.y, speed, health)
{
}

void Enemy::initVisuals() {
    // 1. Menacing Raider Main Hull (Inverted 8-point polygon pointing downward)
    m_hull.setPointCount(8);
    m_hull.setPoint(0, sf::Vector2f(0.0f, 20.0f));    // Forward pointed prow
    m_hull.setPoint(1, sf::Vector2f(10.0f, 6.0f));    // Right lower chin
    m_hull.setPoint(2, sf::Vector2f(21.0f, -12.0f));  // Right swept wingtip
    m_hull.setPoint(3, sf::Vector2f(12.0f, -20.0f));  // Right top stabilizer
    m_hull.setPoint(4, sf::Vector2f(0.0f, -14.0f));   // Center top engine notch
    m_hull.setPoint(5, sf::Vector2f(-12.0f, -20.0f)); // Left top stabilizer
    m_hull.setPoint(6, sf::Vector2f(-21.0f, -12.0f)); // Left swept wingtip
    m_hull.setPoint(7, sf::Vector2f(-10.0f, 6.0f));   // Left lower chin

    m_hull.setFillColor(sf::Color(135, 18, 42));       // Aggressive crimson hull
    m_hull.setOutlineColor(sf::Color(255, 60, 85));    // Glowing neon red/scarlet edge
    m_hull.setOutlineThickness(1.5f);

    // 2. Sinister Cockpit / Scanning Sensor
    m_cockpit.setPointCount(4);
    m_cockpit.setPoint(0, sf::Vector2f(0.0f, 8.0f));   // Lower focus
    m_cockpit.setPoint(1, sf::Vector2f(4.0f, 0.0f));   // Right edge
    m_cockpit.setPoint(2, sf::Vector2f(0.0f, -6.0f));  // Upper edge
    m_cockpit.setPoint(3, sf::Vector2f(-4.0f, 0.0f));  // Left edge

    m_cockpit.setFillColor(sf::Color(255, 130, 40));   // Glowing plasma amber
    m_cockpit.setOutlineColor(sf::Color(255, 230, 120));
    m_cockpit.setOutlineThickness(0.5f);

    // 3. Wing Inset Spikes
    m_leftWing.setPointCount(3);
    m_leftWing.setPoint(0, sf::Vector2f(-9.0f, -4.0f));
    m_leftWing.setPoint(1, sf::Vector2f(-18.0f, -10.0f));
    m_leftWing.setPoint(2, sf::Vector2f(-11.0f, -14.0f));
    m_leftWing.setFillColor(sf::Color(190, 30, 65));

    m_rightWing.setPointCount(3);
    m_rightWing.setPoint(0, sf::Vector2f(9.0f, -4.0f));
    m_rightWing.setPoint(1, sf::Vector2f(18.0f, -10.0f));
    m_rightWing.setPoint(2, sf::Vector2f(11.0f, -14.0f));
    m_rightWing.setFillColor(sf::Color(190, 30, 65));

    // 4. Rear Engine Thruster
    m_thruster.setSize(sf::Vector2f(8.0f, 6.0f));
    m_thruster.setOrigin(4.0f, 6.0f);
    m_thruster.setFillColor(sf::Color(240, 40, 80));
    m_thruster.setOutlineColor(sf::Color(255, 120, 150));
    m_thruster.setOutlineThickness(1.0f);
}

void Enemy::update(sf::Time deltaTime) {
    // Translate downward (increasing Y coordinate)
    m_position.y += m_speed * deltaTime.asSeconds();
    updateVisualPositions();
}

void Enemy::updateVisualPositions() {
    m_hull.setPosition(m_position);
    m_cockpit.setPosition(m_position);
    m_leftWing.setPosition(m_position);
    m_rightWing.setPosition(m_position);
    m_thruster.setPosition(m_position.x, m_position.y - 14.0f);
}

void Enemy::render(sf::RenderTarget& target) const {
    target.draw(m_thruster);
    target.draw(m_hull);
    target.draw(m_leftWing);
    target.draw(m_rightWing);
    target.draw(m_cockpit);
}

bool Enemy::isOffScreen(float bottomBoundary) const {
    // Escaped if top edge has passed past the bottom window boundary
    return (m_position.y - (ENEMY_HEIGHT / 2.0f)) > bottomBoundary;
}

sf::Vector2f Enemy::getPosition() const {
    return m_position;
}

void Enemy::setPosition(const sf::Vector2f& position) {
    m_position = position;
    updateVisualPositions();
}

void Enemy::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    updateVisualPositions();
}

int Enemy::getHealth() const {
    return m_health;
}

void Enemy::setHealth(int health) {
    m_health = std::max(0, health);
}

void Enemy::takeDamage(int amount) {
    if (amount > 0) {
        m_health = std::max(0, m_health - amount);
    }
}

void Enemy::destroy() {
    m_health = 0;
}

bool Enemy::isDestroyed() const {
    return m_health <= 0;
}

float Enemy::getSpeed() const {
    return m_speed;
}

void Enemy::setSpeed(float speed) {
    m_speed = speed;
}

sf::FloatRect Enemy::getBounds() const {
    return sf::FloatRect(
        m_position.x - (ENEMY_WIDTH / 2.0f),
        m_position.y - (ENEMY_HEIGHT / 2.0f),
        ENEMY_WIDTH,
        ENEMY_HEIGHT
    );
}
