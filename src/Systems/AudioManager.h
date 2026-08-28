#pragma once

#include <SDL3/SDL.h>
#include <vector>

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    bool init();
    void shutdown();

    void playSound(float frequency, float duration, bool squareWave = true);
    void playMoveSound();
    void playInvalidMoveSound();
    void playDeathSound();
    void playWinSound();
    void playDestructionSound();
    void playWarningSound();

    void toggleSound() { m_soundOn = !m_soundOn; }
    bool isSoundOn() const { return m_soundOn; }

private:
    SDL_AudioDeviceID m_device = 0;
    SDL_AudioStream* m_stream = nullptr;
    bool m_soundOn = true;
};
