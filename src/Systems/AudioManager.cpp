#include "AudioManager.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
    shutdown();
}

bool AudioManager::init()
{
    // Configure audio spec for mono float playback at 44.1kHz
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq = 44100;

    // Open default playback logical audio device
    m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (m_device == 0)
    {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create a conversion stream (input spec matches output spec)
    m_stream = SDL_CreateAudioStream(&spec, &spec);
    if (!m_stream)
    {
        std::cerr << "Failed to create audio stream: " << SDL_GetError() << std::endl;
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
        return false;
    }

    // Bind stream to logical device for playback
    if (!SDL_BindAudioStream(m_device, m_stream))
    {
        std::cerr << "Failed to bind audio stream: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
        return false;
    }

    // Start playback (starts unpaused, but good practice to resume)
    SDL_ResumeAudioDevice(m_device);

    return true;
}

void AudioManager::shutdown()
{
    if (m_stream)
    {
        SDL_UnbindAudioStream(m_stream);
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
    if (m_device != 0)
    {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
}

void AudioManager::playSound(float frequency, float duration, bool squareWave)
{
    if (!m_soundOn || !m_stream) return;

    // Clear any previously queued audio to prevent latency build-up and stutters
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * frequency * t;
        
        float sampleVal = 0.0f;
        if (squareWave)
        {
            sampleVal = (std::sin(angle) >= 0.0f) ? 0.08f : -0.08f; // Modest volume
        }
        else
        {
            sampleVal = std::sin(angle) * 0.12f;
        }

        // Apply a quick volume envelope (fade out at the end to prevent clicking)
        float fadeOutLimit = 0.8f * numSamples;
        if (i > fadeOutLimit)
        {
            float ratio = (numSamples - i) / (numSamples - fadeOutLimit);
            sampleVal *= ratio;
        }

        samples[i] = sampleVal;
    }

    // Queue audio data for immediate playback
    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playMoveSound()
{
    playSound(587.33f, 0.06f, false); // D5 sine beep
}

void AudioManager::playInvalidMoveSound()
{
    playSound(146.83f, 0.12f, true); // Low D3 buzz
}

void AudioManager::playWarningSound()
{
    playSound(880.00f, 0.07f, true); // High A5 pulse
}

void AudioManager::playDestructionSound()
{
    // Generate a sliding frequency sound (decaying rumble)
    if (!m_soundOn || !m_stream) return;

    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.18f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        // Slide frequency down from 300Hz to 60Hz
        float freq = 300.0f - progress * 240.0f;
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;
        
        float sampleVal = (std::sin(angle) >= 0.0f) ? 0.07f : -0.07f;
        
        // Envelope fade-out
        sampleVal *= (1.0f - progress);

        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playDeathSound()
{
    // Low downward buzz slide
    if (!m_soundOn || !m_stream) return;

    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.45f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        float freq = 250.0f - progress * 200.0f; // Slide down from 250Hz to 50Hz
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;
        
        float sampleVal = (std::sin(angle) >= 0.0f) ? 0.1f : -0.1f;
        
        // Add low frequency noise overlay for crunchiness
        if (i % 7 == 0)
        {
            sampleVal += ((rand() % 100) / 100.0f - 0.5f) * 0.05f;
        }

        sampleVal *= (1.0f - progress);
        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playWinSound()
{
    // Generate a single combined buffer of ascending arpeggio notes
    // C5 -> E5 -> G5 -> C6 to play them sequentially without using SDL_Delay (which freezes/stutters the main thread)
    if (!m_soundOn || !m_stream) return;

    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    struct NoteSpec { float freq; float dur; };
    NoteSpec notes[] = {
        { 523.25f, 0.08f }, // C5
        { 659.25f, 0.08f }, // E5
        { 783.99f, 0.08f }, // G5
        { 1046.50f, 0.22f }  // C6
    };

    std::vector<float> totalSamples;

    for (const auto& note : notes)
    {
        int numSamples = static_cast<int>(note.dur * sampleRate);
        for (int i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float angle = 2.0f * static_cast<float>(M_PI) * note.freq * t;
            
            float sampleVal = std::sin(angle) * 0.12f;

            // Envelope fade out to prevent clicking
            float fadeOutLimit = 0.8f * numSamples;
            if (i > fadeOutLimit)
            {
                float ratio = (numSamples - i) / (numSamples - fadeOutLimit);
                sampleVal *= ratio;
            }
            totalSamples.push_back(sampleVal);
        }
    }

    SDL_PutAudioStreamData(m_stream, totalSamples.data(), static_cast<int>(totalSamples.size() * sizeof(float)));
}
