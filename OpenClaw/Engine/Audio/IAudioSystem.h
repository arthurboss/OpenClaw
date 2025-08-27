#pragma once

#include <string>
#include <cstddef>

// Abstract audio system interface for cross-platform compatibility
class IAudioSystem {
public:
    virtual ~IAudioSystem() = default;

    // Core audio functionality
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    
    // Sound effects
    virtual bool LoadSound(const std::string& name, const char* data, size_t size) = 0;
    virtual bool PlaySound(const std::string& name, float volume = 1.0f) = 0;
    virtual bool PlaySoundWithPath(const std::string& originalPath, const char* data, size_t size, float volume = 1.0f) = 0;
    virtual void StopSound(const std::string& name) = 0;
    virtual void StopAllSounds() = 0;
    
    // Music
    virtual bool LoadMusic(const std::string& name, const char* data, size_t size) = 0;
    virtual bool PlayMusic(const std::string& name, bool looping = false) = 0;
    virtual void StopMusic() = 0;
    virtual void PauseMusic() = 0;
    virtual void ResumeMusic() = 0;
    
    // Volume control
    virtual void SetSoundVolume(float volume) = 0;
    virtual void SetMusicVolume(float volume) = 0;
    virtual float GetSoundVolume() const = 0;
    virtual float GetMusicVolume() const = 0;
    
    // Enable/disable
    virtual void SetSoundEnabled(bool enabled) = 0;
    virtual void SetMusicEnabled(bool enabled) = 0;
    virtual bool IsSoundEnabled() const = 0;
    virtual bool IsMusicEnabled() const = 0;
    
    // Status
    virtual bool IsInitialized() const = 0;
    virtual bool IsMusicPlaying() const = 0;
};
