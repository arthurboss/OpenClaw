#pragma once

#include "IAudioSystem.h"
#include <memory>

// Factory for creating platform-specific audio systems
class AudioSystemFactory {
public:
    enum class AudioSystemType {
        SDL2_MIXER,      // Native SDL2_mixer (Windows, Linux, macOS)
        AUDIO_WORKLET,   // AudioWorklet for WASM builds
        WEB_AUDIO_API    // Web Audio API for WASM builds (fallback)
    };

    // Create the appropriate audio system for the current platform
    static std::unique_ptr<IAudioSystem> CreateAudioSystem(AudioSystemType type = AudioSystemType::SDL2_MIXER);
    
    // Get the recommended audio system type for the current platform
    static AudioSystemType GetRecommendedAudioSystemType();
    
    // Check if a specific audio system type is supported
    static bool IsAudioSystemSupported(AudioSystemType type);
};
