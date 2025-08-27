#pragma once

#include "../IAudioSystem.h"
#include <map>
#include <string>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// AudioWorklet-based audio system for WASM builds
// Uses separate threads for audio processing to avoid conflicts
class AudioWorkletSystem : public IAudioSystem {
private:
    bool m_initialized;
    bool m_soundEnabled;
    bool m_musicEnabled;
    float m_soundVolume;
    float m_musicVolume;
    bool m_musicPlaying;
    
    // Audio buffers storage
    std::map<std::string, std::vector<char>> m_soundBuffers;
    std::map<std::string, std::vector<char>> m_musicBuffers;
    
    // Current playing music
    std::string m_currentMusic;
    bool m_musicLooping;

public:
    AudioWorkletSystem();
    virtual ~AudioWorkletSystem();

    // IAudioSystem implementation
    bool Initialize() override;
    void Shutdown() override;
    
    // Sound effects
    bool LoadSound(const std::string& name, const char* data, size_t size) override;
    bool PlaySound(const std::string& name, float volume = 1.0f) override;
    bool PlaySoundWithPath(const std::string& originalPath, const char* data, size_t size, float volume = 1.0f, int loops = 0) override;
    void StopSound(const std::string& name) override;
    void StopAllSounds() override;
    
    // Music
    bool LoadMusic(const std::string& name, const char* data, size_t size) override;
    bool PlayMusic(const std::string& name, bool looping = false) override;
    void StopMusic() override;
    void PauseMusic() override;
    void ResumeMusic() override;
    
    // Volume control
    void SetSoundVolume(float volume) override;
    void SetMusicVolume(float volume) override;
    float GetSoundVolume() const override { return m_soundVolume; }
    float GetMusicVolume() const override { return m_musicVolume; }
    
    // Enable/disable
    void SetSoundEnabled(bool enabled) override { m_soundEnabled = enabled; }
    void SetMusicEnabled(bool enabled) override;
    bool IsSoundEnabled() const override { return m_soundEnabled; }
    bool IsMusicEnabled() const override { return m_musicEnabled; }
    
    // Status
    bool IsInitialized() const override { return m_initialized; }
    bool IsMusicPlaying() const override { return m_musicPlaying; }

private:
    // AudioWorklet-specific methods
    bool InitializeAudioWorklet();
    bool LoadAudioWorkletScript();
    void SendMessageToAudioWorklet(const std::string& message);
};
