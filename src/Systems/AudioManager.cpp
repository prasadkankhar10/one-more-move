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

    // Create separate stream for background music
    m_bgmStream = SDL_CreateAudioStream(&spec, &spec);
    if (m_bgmStream)
    {
        SDL_BindAudioStream(m_device, m_bgmStream);
    }

    // Start playback (starts unpaused, but good practice to resume)
    SDL_ResumeAudioDevice(m_device);

    return true;
}

void AudioManager::shutdown()
{
    stopBGM();
    if (m_bgmStream)
    {
        SDL_UnbindAudioStream(m_bgmStream);
        SDL_DestroyAudioStream(m_bgmStream);
        m_bgmStream = nullptr;
    }
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

void AudioManager::playSlideSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.08f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        float freq = 400.0f + progress * 400.0f;
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;
        float sampleVal = std::sin(angle) * 0.09f * (1.0f - progress * 0.5f);
        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playPortalSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.22f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        float freq = 300.0f + std::sin(progress * static_cast<float>(M_PI)) * 600.0f;
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;
        float sampleVal = (std::sin(angle) >= 0.0f ? 0.08f : -0.08f) * (1.0f - progress * 0.7f);
        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playKeySound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    struct NoteSpec { float freq; float dur; };
    NoteSpec notes[] = {
        { 783.99f, 0.06f }, // G5
        { 1046.50f, 0.16f } // C6
    };

    std::vector<float> samples;
    for (const auto& note : notes)
    {
        int count = static_cast<int>(note.dur * sampleRate);
        for (int i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float angle = 2.0f * static_cast<float>(M_PI) * note.freq * t;
            float sampleVal = std::sin(angle) * 0.12f * (1.0f - static_cast<float>(i) / count);
            samples.push_back(sampleVal);
        }
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playGateSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.18f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        float freq = 120.0f - progress * 40.0f;
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;
        float sampleVal = (std::sin(angle) >= 0.0f ? 0.09f : -0.09f);
        if (i % 5 == 0)
        {
            sampleVal += ((rand() % 100) / 100.0f - 0.5f) * 0.04f;
        }
        sampleVal *= (1.0f - progress);
        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playBombSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.35f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        float freq = 180.0f - progress * 150.0f; // Down to 30Hz
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;
        float sampleVal = (std::sin(angle) >= 0.0f ? 0.12f : -0.12f);
        // Heavy noise burst
        float noise = ((rand() % 200) / 100.0f - 1.0f) * 0.08f * (1.0f - progress);
        sampleVal = (sampleVal + noise) * (1.0f - progress);
        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playShieldSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    struct NoteSpec { float freq; float dur; };
    NoteSpec notes[] = {
        { 659.25f, 0.06f }, // E5
        { 830.61f, 0.06f }, // G#5
        { 987.77f, 0.14f }  // B5
    };

    std::vector<float> samples;
    for (const auto& note : notes)
    {
        int count = static_cast<int>(note.dur * sampleRate);
        for (int i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float angle = 2.0f * static_cast<float>(M_PI) * note.freq * t;
            float sampleVal = std::sin(angle) * 0.10f * (1.0f - static_cast<float>(i) / count * 0.5f);
            samples.push_back(sampleVal);
        }
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playCoinSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    struct NoteSpec { float freq; float dur; };
    NoteSpec notes[] = {
        { 987.77f, 0.05f },  // B5
        { 1318.51f, 0.20f }  // E6
    };

    std::vector<float> samples;
    for (const auto& note : notes)
    {
        int count = static_cast<int>(note.dur * sampleRate);
        for (int i = 0; i < count; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float angle = 2.0f * static_cast<float>(M_PI) * note.freq * t;
            float sampleVal = (std::sin(angle) >= 0.0f ? 0.08f : -0.08f) * (1.0f - static_cast<float>(i) / count);
            samples.push_back(sampleVal);
        }
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::playFreezeSound()
{
    if (!m_soundOn || !m_stream) return;
    SDL_ClearAudioStream(m_stream);

    const int sampleRate = 44100;
    float duration = 0.25f;
    int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> samples(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float progress = static_cast<float>(i) / numSamples;
        float t = static_cast<float>(i) / sampleRate;
        // Shimmering triple tones
        float s1 = std::sin(2.0f * static_cast<float>(M_PI) * 1200.0f * t);
        float s2 = std::sin(2.0f * static_cast<float>(M_PI) * 1500.0f * t);
        float s3 = std::sin(2.0f * static_cast<float>(M_PI) * 1800.0f * t);
        float sampleVal = ((s1 + s2 + s3) / 3.0f) * 0.09f * (1.0f - progress);
        samples[i] = sampleVal;
    }

    SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)));
}

void AudioManager::stopBGM()
{
    if (m_bgmStream)
    {
        SDL_ClearAudioStream(m_bgmStream);
    }
}

void AudioManager::updateBGM(bool fastTempo, bool isPlaying)
{
    if (!m_soundOn || !m_bgmStream || !isPlaying)
    {
        stopBGM();
        return;
    }

    const int sampleRate = 44100;
    // Check if we need more audio queued (keep ~250ms buffered)
    int queuedBytes = SDL_GetAudioStreamQueued(m_bgmStream);
    int minQueueBytes = static_cast<int>(sampleRate * sizeof(float) * 0.25f);

    if (queuedBytes >= minQueueBytes)
    {
        return;
    }

    // Classic 16-step retro bassline sequence
    static const float s_bgmMelody[16] = {
        110.00f, 130.81f, 164.81f, 220.00f, // A2, C3, E3, A3
        196.00f, 164.81f, 130.81f, 164.81f, // G3, E3, C3, E3
        87.31f,  110.00f, 130.81f, 174.61f, // F2, A2, C3, F3
        98.00f,  123.47f, 146.83f, 196.00f  // G2, B2, D3, G3
    };

    float noteDur = fastTempo ? 0.085f : 0.125f;
    int noteSamples = static_cast<int>(noteDur * sampleRate);
    std::vector<float> chunk(noteSamples);

    float freq = s_bgmMelody[m_bgmStep];
    m_bgmStep = (m_bgmStep + 1) % 16;

    for (int i = 0; i < noteSamples; ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        float angle = 2.0f * static_cast<float>(M_PI) * freq * t;

        // Retro pulse + sine sub-bass mix
        float pulse = (std::sin(angle) >= 0.0f) ? 0.035f : -0.035f;
        float sub = std::sin(angle * 0.5f) * 0.025f;
        float val = pulse + sub;

        // Envelope: punchy staccato pluck
        float decay = 1.0f - (static_cast<float>(i) / noteSamples);
        val *= (decay * decay);

        chunk[i] = val;
    }

    SDL_PutAudioStreamData(m_bgmStream, chunk.data(), static_cast<int>(chunk.size() * sizeof(float)));
}
