#include "AudioSystemFactory.h"
#include <memory>

#ifdef __EMSCRIPTEN__
#include "WASM/AudioWorkletSystem.h"
#endif

// TODO: Add SDL2_mixer implementation when needed
// #include "SDL2/SDL2MixerSystem.h"

std::unique_ptr<IAudioSystem> AudioSystemFactory::CreateAudioSystem(AudioSystemType type) {
    switch (type) {
#ifdef __EMSCRIPTEN__
        case AudioSystemType::AUDIO_WORKLET:
            return std::unique_ptr<AudioWorkletSystem>(new AudioWorkletSystem());
        case AudioSystemType::WEB_AUDIO_API:
            // TODO: Implement Web Audio API system
            return nullptr;
#endif
        case AudioSystemType::SDL2_MIXER:
            // TODO: Implement SDL2_mixer system
            return nullptr;
        default:
            return nullptr;
    }
}

AudioSystemFactory::AudioSystemType AudioSystemFactory::GetRecommendedAudioSystemType() {
#ifdef __EMSCRIPTEN__
    // For WASM builds, prefer AudioWorklet for better performance and thread isolation
    if (IsAudioSystemSupported(AudioSystemType::AUDIO_WORKLET)) {
        return AudioSystemType::AUDIO_WORKLET;
    }
    // Fallback to Web Audio API if AudioWorklet is not supported
    if (IsAudioSystemSupported(AudioSystemType::WEB_AUDIO_API)) {
        return AudioSystemType::WEB_AUDIO_API;
    }
#endif

    // For native builds, use SDL2_mixer
    return AudioSystemType::SDL2_MIXER;
}

bool AudioSystemFactory::IsAudioSystemSupported(AudioSystemType type) {
    switch (type) {
#ifdef __EMSCRIPTEN__
        case AudioSystemType::AUDIO_WORKLET:
            // Check if AudioWorklet is supported in the browser
            return true; // We'll handle the actual check in the implementation
        case AudioSystemType::WEB_AUDIO_API:
            // Check if Web Audio API is supported
            return true; // We'll handle the actual check in the implementation
#endif
        case AudioSystemType::SDL2_MIXER:
            // Check if SDL2_mixer is available
            return true; // We'll handle the actual check in the implementation
        default:
            return false;
    }
}
