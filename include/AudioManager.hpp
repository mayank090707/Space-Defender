#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <vector>

/**
 * @brief Sound event types in Space Defender.
 */
enum class SoundEffect {
    Shoot,
    Explosion,
    PlayerDamage,
    EnemyEscape,
    GameOver,
    MenuSelect
};

/**
 * @brief Manages procedural sound synthesis, audio playback, volume levels,
 *        and sound muting for Space Defender using SFML Audio.
 */
class AudioManager {
public:
    static constexpr unsigned int SAMPLE_RATE = 44100;

    AudioManager();
    ~AudioManager() = default;

    // Prevent copying
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    /**
     * @brief Initializes sound buffers via procedural waveform synthesis
     *        or loads from assets/sounds if available.
     */
    void init();

    void playSound(SoundEffect effect);

    void playShoot();
    void playExplosion();
    void playPlayerDamage();
    void playEnemyEscape();
    void playGameOver();
    void playMenuSelect();

    void toggleMute();
    void setMuted(bool muted);
    [[nodiscard]] bool isMuted() const;

    void setVolume(float volume);
    [[nodiscard]] float getVolume() const;

private:
    void generateProceduralAudio();
    void synthesizeShootBuffer();
    void synthesizeExplosionBuffer();
    void synthesizePlayerDamageBuffer();
    void synthesizeEnemyEscapeBuffer();
    void synthesizeGameOverBuffer();
    void synthesizeMenuSelectBuffer();

    bool m_muted;
    float m_volume;
    bool m_initialized;

    // Sound buffers
    sf::SoundBuffer m_shootBuffer;
    sf::SoundBuffer m_explosionBuffer;
    sf::SoundBuffer m_damageBuffer;
    sf::SoundBuffer m_escapeBuffer;
    sf::SoundBuffer m_gameOverBuffer;
    sf::SoundBuffer m_menuSelectBuffer;

    // Active sound instances
    sf::Sound m_shootSound;
    sf::Sound m_explosionSound;
    sf::Sound m_damageSound;
    sf::Sound m_escapeSound;
    sf::Sound m_gameOverSound;
    sf::Sound m_menuSelectSound;

    sf::Clock m_shootAudioThrottle;
};
