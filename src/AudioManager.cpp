#include "AudioManager.hpp"
#include <cmath>
#include <random>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioManager::AudioManager()
    : m_muted(false)
    , m_volume(80.0f)
    , m_initialized(false)
{
    init();
}

void AudioManager::init() {
    if (m_initialized) {
        return;
    }
    m_initialized = true;

    generateProceduralAudio();

    m_shootSound.setBuffer(m_shootBuffer);
    m_explosionSound.setBuffer(m_explosionBuffer);
    m_damageSound.setBuffer(m_damageBuffer);
    m_escapeSound.setBuffer(m_escapeBuffer);
    m_gameOverSound.setBuffer(m_gameOverBuffer);
    m_menuSelectSound.setBuffer(m_menuSelectBuffer);

    setVolume(m_volume);
    std::cout << "[AudioManager] Procedural 8-bit retro sound synthesizers initialized." << std::endl;
}

void AudioManager::generateProceduralAudio() {
    synthesizeShootBuffer();
    synthesizeExplosionBuffer();
    synthesizePlayerDamageBuffer();
    synthesizeEnemyEscapeBuffer();
    synthesizeGameOverBuffer();
    synthesizeMenuSelectBuffer();
}

void AudioManager::synthesizeShootBuffer() {
    // 0.10s frequency sweep from 950 Hz down to 220 Hz
    const unsigned int sampleCount = static_cast<unsigned int>(SAMPLE_RATE * 0.10f);
    std::vector<sf::Int16> samples(sampleCount);

    double phase = 0.0;
    for (unsigned int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double progress = t / 0.10;
        double frequency = 950.0 - (progress * 730.0);
        double envelope = 1.0 - progress; // Linear fade out

        phase += (2.0 * M_PI * frequency) / SAMPLE_RATE;
        double value = std::sin(phase) * envelope * 24000.0;
        samples[i] = static_cast<sf::Int16>(value);
    }
    m_shootBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void AudioManager::synthesizeExplosionBuffer() {
    // 0.35s decaying white noise burst
    const unsigned int sampleCount = static_cast<unsigned int>(SAMPLE_RATE * 0.35f);
    std::vector<sf::Int16> samples(sampleCount);

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (unsigned int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double envelope = std::exp(-t * 9.5); // Exponential decay
        double noise = dist(rng);
        double value = noise * envelope * 28000.0;
        samples[i] = static_cast<sf::Int16>(value);
    }
    m_explosionBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void AudioManager::synthesizePlayerDamageBuffer() {
    // 0.22s low-frequency crunch / buzz (130 Hz down to 50 Hz)
    const unsigned int sampleCount = static_cast<unsigned int>(SAMPLE_RATE * 0.22f);
    std::vector<sf::Int16> samples(sampleCount);

    std::mt19937 rng(1337);
    std::uniform_real_distribution<double> noiseDist(-0.25, 0.25);

    double phase = 0.0;
    for (unsigned int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double progress = t / 0.22;
        double frequency = 130.0 - (progress * 80.0);
        double envelope = std::cos(progress * M_PI * 0.5);

        phase += (2.0 * M_PI * frequency) / SAMPLE_RATE;
        // Square wave with noise distortion
        double wave = (std::sin(phase) > 0.0 ? 0.75 : -0.75) + noiseDist(rng);
        double value = wave * envelope * 26000.0;
        samples[i] = static_cast<sf::Int16>(value);
    }
    m_damageBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void AudioManager::synthesizeEnemyEscapeBuffer() {
    // 0.28s descending warning tone
    const unsigned int sampleCount = static_cast<unsigned int>(SAMPLE_RATE * 0.28f);
    std::vector<sf::Int16> samples(sampleCount);

    double phase = 0.0;
    for (unsigned int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double frequency = (t < 0.14) ? 440.0 : 293.66; // A4 then D4
        double envelope = 1.0 - (t / 0.28);

        phase += (2.0 * M_PI * frequency) / SAMPLE_RATE;
        double value = std::sin(phase) * envelope * 20000.0;
        samples[i] = static_cast<sf::Int16>(value);
    }
    m_escapeBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void AudioManager::synthesizeGameOverBuffer() {
    // 0.65s minor descending arpeggio (E4 -> C4 -> A3 -> F#3)
    const unsigned int sampleCount = static_cast<unsigned int>(SAMPLE_RATE * 0.65f);
    std::vector<sf::Int16> samples(sampleCount);

    const double notes[4] = { 329.63, 261.63, 220.00, 185.00 }; // E4, C4, A3, F#3
    double phase = 0.0;

    for (unsigned int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        int noteIdx = std::min(3, static_cast<int>(t / 0.16));
        double frequency = notes[noteIdx];
        double envelope = 1.0 - (t / 0.65);

        phase += (2.0 * M_PI * frequency) / SAMPLE_RATE;
        double value = std::sin(phase) * envelope * 22000.0;
        samples[i] = static_cast<sf::Int16>(value);
    }
    m_gameOverBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void AudioManager::synthesizeMenuSelectBuffer() {
    // 0.05s bright crisp UI blip (880 Hz A5)
    const unsigned int sampleCount = static_cast<unsigned int>(SAMPLE_RATE * 0.05f);
    std::vector<sf::Int16> samples(sampleCount);

    double phase = 0.0;
    for (unsigned int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double envelope = 1.0 - (t / 0.05);

        phase += (2.0 * M_PI * 880.0) / SAMPLE_RATE;
        double value = std::sin(phase) * envelope * 18000.0;
        samples[i] = static_cast<sf::Int16>(value);
    }
    m_menuSelectBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void AudioManager::playSound(SoundEffect effect) {
    if (m_muted) return;

    switch (effect) {
        case SoundEffect::Shoot: playShoot(); break;
        case SoundEffect::Explosion: playExplosion(); break;
        case SoundEffect::PlayerDamage: playPlayerDamage(); break;
        case SoundEffect::EnemyEscape: playEnemyEscape(); break;
        case SoundEffect::GameOver: playGameOver(); break;
        case SoundEffect::MenuSelect: playMenuSelect(); break;
    }
}

void AudioManager::playShoot() {
    if (m_muted) return;
    // Throttle to prevent acoustic clipping during rapid continuous fire
    if (m_shootAudioThrottle.getElapsedTime().asSeconds() >= 0.12f) {
        m_shootSound.play();
        m_shootAudioThrottle.restart();
    }
}

void AudioManager::playExplosion() {
    if (m_muted) return;
    m_explosionSound.play();
}

void AudioManager::playPlayerDamage() {
    if (m_muted) return;
    m_damageSound.play();
}

void AudioManager::playEnemyEscape() {
    if (m_muted) return;
    m_escapeSound.play();
}

void AudioManager::playGameOver() {
    if (m_muted) return;
    m_gameOverSound.play();
}

void AudioManager::playMenuSelect() {
    if (m_muted) return;
    m_menuSelectSound.play();
}

void AudioManager::toggleMute() {
    m_muted = !m_muted;
    std::cout << "[AudioManager] Sound " << (m_muted ? "MUTED" : "UNMUTED") << std::endl;
}

void AudioManager::setMuted(bool muted) {
    m_muted = muted;
}

bool AudioManager::isMuted() const {
    return m_muted;
}

void AudioManager::setVolume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 100.0f);
    m_shootSound.setVolume(m_volume);
    m_explosionSound.setVolume(m_volume);
    m_damageSound.setVolume(m_volume);
    m_escapeSound.setVolume(m_volume);
    m_gameOverSound.setVolume(m_volume);
    m_menuSelectSound.setVolume(m_volume);
}

float AudioManager::getVolume() const {
    return m_volume;
}
