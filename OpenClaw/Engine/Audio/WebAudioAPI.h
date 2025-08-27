#pragma once

#ifdef __EMSCRIPTEN__

#include <cstddef>  // for size_t

// Web Audio API interface for Emscripten builds
extern "C" {
    bool WebAudio_Initialize();
    bool WebAudio_LoadSound(const char* name, const char* data, size_t size);
    bool WebAudio_PlaySound(const char* name, float volume);
    bool WebAudio_PlayMusic(const char* name, bool looping);
    void WebAudio_StopMusic();
    void WebAudio_PauseMusic();
    void WebAudio_ResumeMusic();
    void WebAudio_SetSoundVolume(float volume);
    void WebAudio_SetMusicVolume(float volume);
    void WebAudio_SetSoundEnabled(bool enabled);
    void WebAudio_SetMusicEnabled(bool enabled);
    void WebAudio_StopAllSounds();
}

#endif // __EMSCRIPTEN__
