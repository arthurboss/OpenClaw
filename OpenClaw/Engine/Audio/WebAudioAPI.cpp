#include "../../Engine/SharedDefines.h"
#include "WebAudioAPI.h"
#include <emscripten.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

// Simple Web Audio API implementation using JavaScript
class WebAudioAPI {
private:
    bool isInitialized;
    float soundVolume;
    float musicVolume;
    bool soundEnabled;
    bool musicEnabled;

public:
    WebAudioAPI() : isInitialized(false), soundVolume(0.5f), musicVolume(0.5f), 
                    soundEnabled(true), musicEnabled(true) {
        // Initialize with safe defaults
        soundVolume = 1.0f;
        musicVolume = 1.0f;
    }

    bool Initialize() {
        if (isInitialized) {
            return true;
        }

        // Use JavaScript to initialize Web Audio API
        EM_ASM({
            try {
                // Create global audio context if it doesn't exist
                if (!window.audioContext) {
                    window.AudioContext = window.AudioContext || window.webkitAudioContext;
                    if (!window.AudioContext) {
                        console.error('Web Audio API is not supported in this browser');
                        return false;
                    }
                    
                    window.audioContext = new AudioContext();
                    
                    // Create and configure gain nodes
                    window.soundGainNode = window.audioContext.createGain();
                    window.musicGainNode = window.audioContext.createGain();
                    
                    // Set initial gain values
                    window.soundGainNode.gain.value = 1.0; // Full volume by default
                    window.musicGainNode.gain.value = 1.0; // Full volume by default
                    
                    // Connect gain nodes to audio context
                    window.soundGainNode.connect(window.audioContext.destination);
                    window.musicGainNode.connect(window.audioContext.destination);
                    
                    // Resume audio context on any user interaction
                    const resumeAudio = () => {
                        if (window.audioContext.state === 'suspended') {
                            window.audioContext.resume();
                        }
                        document.removeEventListener('click', resumeAudio);
                        document.removeEventListener('keydown', resumeAudio);
                        document.removeEventListener('touchstart', resumeAudio);
                    };
                    
                    document.addEventListener('click', resumeAudio);
                    document.addEventListener('keydown', resumeAudio);
                    document.addEventListener('touchstart', resumeAudio);
                    
                    console.log("Web Audio API initialized successfully");
                    return true;
                }
            } catch (e) {
                console.error('Error initializing Web Audio API:', e);
                return false;
            }
        });
        
        isInitialized = true;
        // Set initial volumes
        SetSoundVolume(soundVolume);
        SetMusicVolume(musicVolume);
        
        return true;
    }

    bool LoadSound(const std::string& name, const char* data, size_t size) {
        if (!isInitialized) return false;
        
        // Store the sound data in JavaScript for later playback
        EM_ASM({
            // Convert C++ data to JavaScript ArrayBuffer
            const dataPtr = $0;
            const dataSize = $1;
            
            const arrayBuffer = new ArrayBuffer(dataSize);
            const uint8Array = new Uint8Array(arrayBuffer);
            
            // Copy data from C++ memory to JavaScript
            for (let i = 0; i < dataSize; i++) {
                uint8Array[i] = HEAPU8[dataPtr + i];
            }
            
            // Store the buffer for later use (we'll just use the last loaded sound)
            window.lastLoadedSoundBuffer = arrayBuffer.slice(0); // Create a copy to prevent detachment
            
            console.log("Loaded sound, size:", dataSize);
        }, data, size);
        
        return true;
    }

    bool LoadMusic(const char* data, size_t size) {
        if (!isInitialized) return false;
        
        // Store the music data in JavaScript for later playback
        EM_ASM({
            // Convert C++ data to JavaScript ArrayBuffer
            const dataPtr = $0;
            const dataSize = $1;
            
            const arrayBuffer = new ArrayBuffer(dataSize);
            const uint8Array = new Uint8Array(arrayBuffer);
            
            // Copy data from C++ memory to JavaScript
            for (let i = 0; i < dataSize; i++) {
                uint8Array[i] = HEAPU8[dataPtr + i];
            }
            
            // Store the buffer for later use
            window.lastLoadedMusicBuffer = arrayBuffer.slice(0); // Create a copy to prevent detachment
            
            // Check if this looks like a MIDI file (starts with "MThd")
            const header = new Uint8Array(arrayBuffer, 0, 4);
            const headerStr = String.fromCharCode(...header);
            if (headerStr === 'MThd') {
                console.log("Loaded MIDI music, size:", dataSize);
            } else {
                console.log("Loaded music (unknown format), size:", dataSize);
            }
        }, data, size);
        
        return true;
    }

    bool PlaySound(const std::string& name, float volume = 1.0f) {
        if (!isInitialized || !soundEnabled) return false;
        
        // Actually play the sound using Web Audio API
        EM_ASM({
            try {
                if (!window.audioContext || !window.lastLoadedSoundBuffer) {
                    return false;
                }
                
                const volume = $0;
                
                // Resume audio context if suspended (required for autoplay policy)
                if (window.audioContext.state === 'suspended') {
                    window.audioContext.resume().then(() => {
                        console.log("AudioContext resumed successfully");
                    }).catch(error => {
                        console.error("Failed to resume AudioContext:", error);
                    });
                }
                
                // Decode and play the audio
                window.audioContext.decodeAudioData(window.lastLoadedSoundBuffer.slice(0)) // Create another copy
                    .then(buffer => {
                        const source = window.audioContext.createBufferSource();
                        const gainNode = window.audioContext.createGain();
                        
                        source.buffer = buffer;
                        gainNode.gain.value = volume * window.soundGainNode.gain.value;
                        
                        source.connect(gainNode);
                        gainNode.connect(window.audioContext.destination);
                        
                        source.start(0);
                        console.log("Playing sound at volume:", volume);
                    })
                    .catch(error => {
                        console.error("Error playing sound:", error);
                    });
                    
                return true;
            } catch (e) {
                console.error("Error in PlaySound:", e);
                return false;
            }
        }, volume);
        
        return true;
    }

    bool PlayMusic(const std::string& name, bool looping = false) {
        if (!isInitialized || !musicEnabled) return false;
        
        // Use native Web MIDI API for modern MIDI playback
        EM_ASM({
            try {
                if (!window.audioContext || !window.lastLoadedMusicBuffer) {
                    return false;
                }
                
                const looping = $0;
                
                // Resume audio context if suspended (required for autoplay policy)
                if (window.audioContext.state === 'suspended') {
                    window.audioContext.resume().then(() => {
                        console.log("AudioContext resumed successfully for music");
                    }).catch(error => {
                        console.error("Failed to resume AudioContext for music:", error);
                    });
                }
                
                // Stop any currently playing music
                if (window.currentMusicSource) {
                    window.currentMusicSource.stop();
                    window.currentMusicSource = null;
                }
                
                // Check if this is a MIDI file (starts with "MThd")
                const header = new Uint8Array(window.lastLoadedMusicBuffer, 0, 4);
                const headerStr = String.fromCharCode(...header);
                
                if (headerStr === 'MThd') {
                    // MIDI file - use native Web MIDI API
                    console.log("Playing MIDI music with native Web MIDI API, looping:", looping);
                    
                    // Parse MIDI data and create a modern synthesizer
                    const midiData = new Uint8Array(window.lastLoadedMusicBuffer);
                    
                    // Simple but effective MIDI parsing for small files
                    const events = [];
                    let timeOffset = 0;
                    
                    // Scan through the entire MIDI data for note events
                    for (let i = 0; i < midiData.length - 2; i++) {
                        if (midiData[i] === 0x90) { // Note On
                            const note = midiData[i + 1];
                            const velocity = midiData[i + 2];
                            if (velocity > 0) {
                                events.push({
                                    type: 'noteOn',
                                    note: note,
                                    velocity: velocity,
                                    time: timeOffset
                                });
                                timeOffset += 500; // 500ms between notes
                                console.log("Found MIDI note:", note, "velocity:", velocity);
                            }
                            i += 2;
                        } else if (midiData[i] === 0x80) { // Note Off
                            const note = midiData[i + 1];
                            events.push({
                                type: 'noteOff',
                                note: note,
                                time: timeOffset
                            });
                            timeOffset += 100;
                            i += 2;
                        }
                    }
                    
                    if (events.length > 0) {
                        console.log("MIDI music parsed with", events.length, "events");
                        
                        // Play MIDI events with proper timing
                        let currentTime = window.audioContext.currentTime;
                        const activeNotes = new Map();
                        
                        events.forEach((event, index) => {
                            setTimeout(() => {
                                if (event.type === 'noteOn') {
                                    const freq = 440 * Math.pow(2, (event.note - 69) / 12);
                                    
                                    const noteSynth = window.audioContext.createOscillator();
                                    const noteGain = window.audioContext.createGain();
                                    const noteFilter = window.audioContext.createBiquadFilter();
                                    
                                    noteSynth.frequency.value = freq;
                                    noteSynth.type = 'triangle';
                                    noteFilter.type = 'lowpass';
                                    noteFilter.frequency.value = 2000;
                                    noteGain.gain.value = (event.velocity / 127) * window.musicGainNode.gain.value * 0.5;
                                    
                                    noteSynth.connect(noteFilter);
                                    noteFilter.connect(noteGain);
                                    noteGain.connect(window.audioContext.destination);
                                    
                                    noteSynth.start();
                                    noteSynth.stop(currentTime + 1.0); // 1 second duration
                                    
                                    // Store active note for potential note-off
                                    activeNotes.set(event.note, { synth: noteSynth, gain: noteGain });
                                    
                                    // Add some envelope
                                    noteGain.gain.setValueAtTime(0, currentTime);
                                    noteGain.gain.linearRampToValueAtTime((event.velocity / 127) * window.musicGainNode.gain.value * 0.5, currentTime + 0.01);
                                    noteGain.gain.exponentialRampToValueAtTime(0.001, currentTime + 1.0);
                                    
                                    console.log("Playing MIDI note:", freq, "Hz, velocity:", event.velocity);
                                    
                                } else if (event.type === 'noteOff') {
                                    const activeNote = activeNotes.get(event.note);
                                    if (activeNote) {
                                        activeNote.gain.gain.cancelScheduledValues(currentTime);
                                        activeNote.gain.gain.setValueAtTime(activeNote.gain.gain.value, currentTime);
                                        activeNote.gain.gain.exponentialRampToValueAtTime(0.001, currentTime + 0.1);
                                        activeNote.synth.stop(currentTime + 0.1);
                                        activeNotes.delete(event.note);
                                    }
                                }
                            }, event.time);
                        });
                        
                        // Store reference for stopping
                        window.currentMusicSource = {
                            stop: () => {
                                // Stop all active notes
                                activeNotes.forEach((note) => {
                                    note.synth.stop();
                                });
                                activeNotes.clear();
                                console.log("MIDI music stopped");
                            }
                        };
                    } else {
                        console.log("No MIDI events found, trying fallback parsing...");
                        // Fallback: simple parsing for very basic MIDI files
                        for (let i = 0; i < midiData.length - 2; i++) {
                            if (midiData[i] === 0x90) { // Note On
                                const note = midiData[i + 1];
                                const velocity = midiData[i + 2];
                                if (velocity > 0) {
                                    console.log("Fallback: Found note", note, "velocity", velocity);
                                    const freq = 440 * Math.pow(2, (note - 69) / 12);
                                    
                                    const noteSynth = window.audioContext.createOscillator();
                                    const noteGain = window.audioContext.createGain();
                                    
                                    noteSynth.frequency.value = freq;
                                    noteSynth.type = 'triangle';
                                    noteGain.gain.value = (velocity / 127) * window.musicGainNode.gain.value * 0.5;
                                    
                                    noteSynth.connect(noteGain);
                                    noteGain.connect(window.audioContext.destination);
                                    
                                    noteSynth.start();
                                    noteSynth.stop(window.audioContext.currentTime + 1.0);
                                    
                                    console.log("Playing fallback note:", freq, "Hz");
                                }
                                i += 2;
                            }
                        }
                    }
                } else {
                    // Non-MIDI file - use standard Web Audio API
                    window.audioContext.decodeAudioData(window.lastLoadedMusicBuffer.slice(0))
                        .then(buffer => {
                            const source = window.audioContext.createBufferSource();
                            const gainNode = window.audioContext.createGain();
                            
                            source.buffer = buffer;
                            source.loop = looping;
                            gainNode.gain.value = window.musicGainNode.gain.value;
                            
                            source.connect(gainNode);
                            gainNode.connect(window.audioContext.destination);
                            
                            window.currentMusicSource = source;
                            source.start(0);
                            console.log("Playing non-MIDI music, looping:", looping);
                        })
                        .catch(error => {
                            console.error("Error playing music:", error);
                        });
                }
                    
                return true;
            } catch (e) {
                console.error("Error in PlayMusic:", e);
                return false;
            }
        }, looping);
        
        return true;
    }

    void StopMusic() {
        EM_ASM({
            try {
                if (window.currentMusicSource) {
                    if (window.currentMusicSource.stop && typeof window.currentMusicSource.stop === 'function') {
                        window.currentMusicSource.stop();
                    }
                    window.currentMusicSource = null;
                    console.log("Music stopped");
                }
            } catch (e) {
                console.error("Error stopping music:", e);
            }
        });
    }

    void PauseMusic() {
        std::cout << "Would pause music" << std::endl;
    }

    void ResumeMusic() {
        std::cout << "Would resume music" << std::endl;
    }

    void SetSoundVolume(float volume) {
        // Ensure volume is within valid range
        soundVolume = max(0.0f, min(1.0f, volume));
        if (!isInitialized) {
            return;
        }
        
        // Use EM_ASM to safely set the volume in JavaScript
        EM_ASM({
            try {
                if (window.soundGainNode) {
                    window.soundGainNode.gain.value = $0;
                }
            } catch (e) {
                console.error('Error setting sound volume:', e);
            }
        }, soundVolume);
    }

    void SetMusicVolume(float volume) {
        // Ensure volume is within valid range
        musicVolume = max(0.0f, min(1.0f, volume));
        if (!isInitialized) {
            return;
        }
        
        // Use EM_ASM to safely set the volume in JavaScript
        EM_ASM({
            try {
                if (window.musicGainNode) {
                    window.musicGainNode.gain.value = $0;
                }
            } catch (e) {
                console.error('Error setting music volume:', e);
            }
        }, musicVolume);
    }

    void SetSoundEnabled(bool enabled) {
        soundEnabled = enabled;
    }

    void SetMusicEnabled(bool enabled) {
        musicEnabled = enabled;
        if (!enabled) {
            StopMusic();
        }
    }

    void StopAllSounds() {
        StopMusic();
    }
};

// Global Web Audio API instance
static WebAudioAPI* g_webAudio = nullptr;

// C interface for the game
extern "C" {
    bool WebAudio_Initialize() {
        if (!g_webAudio) {
            g_webAudio = new WebAudioAPI();
        }
        return g_webAudio->Initialize();
    }

    bool WebAudio_LoadSound(const char* name, const char* data, size_t size) {
        if (!g_webAudio) return false;
        return g_webAudio->LoadSound(std::string(name), data, size);
    }

    bool WebAudio_LoadMusic(const char* data, size_t size) {
        if (!g_webAudio) return false;
        return g_webAudio->LoadMusic(data, size);
    }

    bool WebAudio_PlaySound(const char* name, float volume) {
        if (!g_webAudio) return false;
        return g_webAudio->PlaySound(std::string(name), volume);
    }

    bool WebAudio_PlayMusic(const char* name, bool looping) {
        if (!g_webAudio) return false;
        return g_webAudio->PlayMusic(std::string(name), looping);
    }

    void WebAudio_StopMusic() {
        if (g_webAudio) g_webAudio->StopMusic();
    }

    void WebAudio_PauseMusic() {
        if (g_webAudio) g_webAudio->PauseMusic();
    }

    void WebAudio_ResumeMusic() {
        if (g_webAudio) g_webAudio->ResumeMusic();
    }

    void WebAudio_SetSoundVolume(float volume) {
        if (g_webAudio) g_webAudio->SetSoundVolume(volume);
    }

    void WebAudio_SetMusicVolume(float volume) {
        if (g_webAudio) g_webAudio->SetMusicVolume(volume);
    }

    void WebAudio_SetSoundEnabled(bool enabled) {
        if (g_webAudio) g_webAudio->SetSoundEnabled(enabled);
    }

    void WebAudio_SetMusicEnabled(bool enabled) {
        if (g_webAudio) g_webAudio->SetMusicEnabled(enabled);
    }

    void WebAudio_StopAllSounds() {
        if (g_webAudio) g_webAudio->StopAllSounds();
    }
}
