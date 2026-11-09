#include "Player.hpp"
#include <cmath>
#include <algorithm>

Player::Player(float startX, float startY)
    : m_position(startX, startY)
    , m_direction(0.0f, 0.0f)
    , m_speed(DEFAULT_SPEED)
    , m_health(DEFAULT_HEALTH)
{
    initVisuals();
    updateVisualPositions();
}

void Player::initVisuals() {
    // 1. Futuristic Starfighter Main Hull (8-point polygon)
    m_hull.setPointCount(8);
    m_hull.setPoint(0, sf::Vector2f(0.0f, -24.0f));   // Nose tip
    m_hull.setPoint(1, sf::Vector2f(8.0f, -6.0f));    // Forward fuselage
    m_hull.setPoint(2, sf::Vector2f(23.0f, 16.0f));   // Right wingtip
    m_hull.setPoint(3, sf::Vector2f(8.0f, 12.0f));    // Right engine mount
    m_hull.setPoint(4, sf::Vector2f(0.0f, 16.0f));    // Center aft notch
    m_hull.setPoint(5, sf::Vector2f(-8.0f, 12.0f));   // Left engine mount
    m_hull.setPoint(6, sf::Vector2f(-23.0f, 16.0f));  // Left wingtip
    m_hull.setPoint(7, sf::Vector2f(-8.0f, -6.0f));   // Left forward fuselage

    m_hull.setFillColor(sf::Color(28, 42, 68));           // Deep navy hull
    m_hull.setOutlineColor(sf::Color(0, 210, 255));        // Glowing neon cyan border
    m_hull.setOutlineThickness(1.5f);

    // 2. Cockpit Canopy (Aerodynamic diamond)
    m_cockpit.setPointCount(4);
    m_cockpit.setPoint(0, sf::Vector2f(0.0f, -14.0f));  // Front apex
    m_cockpit.setPoint(1, sf::Vector2f(4.0f, -4.0f));   // Right edge
    m_cockpit.setPoint(2, sf::Vector2f(0.0f, 3.0f));    // Rear apex
    m_cockpit.setPoint(3, sf::Vector2f(-4.0f, -4.0f));  // Left edge

    m_cockpit.setFillColor(sf::Color(110, 240, 255));     // Luminous sky cyan
    m_cockpit.setOutlineColor(sf::Color(200, 255, 255));
    m_cockpit.setOutlineThickness(0.5f);

    // 3. Wing Accents (Aerodynamic stabilizer highlights)
    m_leftWingAccent.setPointCount(3);
    m_leftWingAccent.setPoint(0, sf::Vector2f(-10.0f, 4.0f));
    m_leftWingAccent.setPoint(1, sf::Vector2f(-20.0f, 14.0f));
    m_leftWingAccent.setPoint(2, sf::Vector2f(-12.0f, 12.0f));
    m_leftWingAccent.setFillColor(sf::Color(0, 180, 230));

    m_rightWingAccent.setPointCount(3);
    m_rightWingAccent.setPoint(0, sf::Vector2f(10.0f, 4.0f));
    m_rightWingAccent.setPoint(1, sf::Vector2f(20.0f, 14.0f));
    m_rightWingAccent.setPoint(2, sf::Vector2f(12.0f, 12.0f));
    m_rightWingAccent.setFillColor(sf::Color(0, 180, 230));

    // 4. Plasma Thruster Exhaust
    m_thruster.setSize(sf::Vector2f(8.0f, 8.0f));
    m_thruster.setOrigin(4.0f, 0.0f);
    m_thruster.setFillColor(sf::Color(255, 145, 30));     // Plasma orange
    m_thruster.setOutlineColor(sf::Color(255, 220, 100));
    m_thruster.setOutlineThickness(1.0f);
}

void Player::handleInput() {
    m_direction = {0.0f, 0.0f};

    // Upward movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        m_direction.y -= 1.0f;
    }
    // Downward movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        m_direction.y += 1.0f;
    }
    // Leftward movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        m_direction.x -= 1.0f;
    }
    // Rightward movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        m_direction.x += 1.0f;
    }

    // Normalization to maintain uniform speed across all 8 movement directions
    float lengthSquared = m_direction.x * m_direction.x + m_direction.y * m_direction.y;
    if (lengthSquared > 0.0f) {
        float invLength = 1.0f / std::sqrt(lengthSquared);
        m_direction.x *= invLength;
        m_direction.y *= invLength;
    }
}

void Player::update(sf::Time deltaTime, const sf::Vector2u& windowSize) {
    // Delta-time based frame-rate independent position translation
    m_position += m_direction * (m_speed * deltaTime.asSeconds());

    // Window boundary collision clamp
    clampToBounds(windowSize);

    // Synchronize all visual sub-components to the new position
    updateVisualPositions();
}

void Player::clampToBounds(const sf::Vector2u& windowSize) {
    const float halfWidth = SHIP_WIDTH / 2.0f;
    const float halfHeight = SHIP_HEIGHT / 2.0f;

    const float minX = halfWidth;
    const float maxX = static_cast<float>(windowSize.x) - halfWidth;
    const float minY = halfHeight;
    const float maxY = static_cast<float>(windowSize.y) - halfHeight;

    m_position.x = std::clamp(m_position.x, minX, maxX);
    m_position.y = std::clamp(m_position.y, minY, maxY);
}

void Player::updateVisualPositions() {
    m_hull.setPosition(m_position);
    m_cockpit.setPosition(m_position);
    m_leftWingAccent.setPosition(m_position);
    m_rightWingAccent.setPosition(m_position);
    m_thruster.setPosition(m_position.x, m_position.y + 14.0f);
}

void Player::render(sf::RenderTarget& target) const {
    target.draw(m_thruster);
    target.draw(m_hull);
    target.draw(m_leftWingAccent);
    target.draw(m_rightWingAccent);
    target.draw(m_cockpit);
}

sf::Vector2f Player::getPosition() const {
    return m_position;
}

sf::Vector2f Player::getMuzzlePosition() const {
    // The nose tip of the starfighter is at Y - (SHIP_HEIGHT / 2)
    return sf::Vector2f(m_position.x, m_position.y - (SHIP_HEIGHT / 2.0f));
}

void Player::setPosition(const sf::Vector2f& position) {
    m_position = position;
    updateVisualPositions();
}

void Player::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    updateVisualPositions();
}

int Player::getHealth() const {
    return m_health;
}

void Player::setHealth(int health) {
    m_health = std::max(0, health);
}

void Player::takeDamage(int amount) {
    if (amount > 0) {
        m_health = std::max(0, m_health - amount);
    }
}

bool Player::isAlive() const {
    return m_health > 0;
}

float Player::getSpeed() const {
    return m_speed;
}

void Player::setSpeed(float speed) {
    m_speed = speed;
}

sf::FloatRect Player::getBounds() const {
    const float halfWidth = SHIP_WIDTH / 2.0f;
    const float halfHeight = SHIP_HEIGHT / 2.0f;
    return sf::FloatRect(m_position.x - halfWidth, m_position.y - halfHeight, SHIP_WIDTH, SHIP_HEIGHT);
}
