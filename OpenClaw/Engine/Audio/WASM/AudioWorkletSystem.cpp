#include "AudioWorkletSystem.h"
#include <iostream>
#include <algorithm>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#endif

AudioWorkletSystem::AudioWorkletSystem()
    : m_initialized(false)
    , m_soundEnabled(true)
    , m_musicEnabled(true)
    , m_soundVolume(1.0f)
    , m_musicVolume(1.0f)
    , m_musicPlaying(false)
    , m_musicLooping(false) {
}

AudioWorkletSystem::~AudioWorkletSystem() {
    Shutdown();
}

bool AudioWorkletSystem::Initialize() {
    if (m_initialized) {
        return true;
    }

#ifdef __EMSCRIPTEN__
    // Initialize SDL2_mixer for audio decoding
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL2_mixer: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Initialize Web Audio API directly (simpler approach)
    if (!InitializeAudioWorklet()) {
        std::cerr << "Failed to initialize Web Audio API" << std::endl;
        Mix_CloseAudio();
        return false;
    }
    
    // Skip AudioWorklet for now, use direct Web Audio API
    m_initialized = true;
    std::cout << "Web Audio API system initialized successfully" << std::endl;
    return true;
#endif

    m_initialized = true;
    std::cout << "AudioWorklet system initialized successfully" << std::endl;
    return true;
}

void AudioWorkletSystem::Shutdown() {
    if (!m_initialized) {
        return;
    }

    StopAllSounds();
    StopMusic();
    
    m_soundBuffers.clear();
    m_musicBuffers.clear();
    m_initialized = false;
    
#ifdef __EMSCRIPTEN__
    Mix_CloseAudio();
#endif
    
    std::cout << "AudioWorklet system shutdown" << std::endl;
}

bool AudioWorkletSystem::LoadSound(const std::string& name, const char* data, size_t size) {
    if (!m_initialized || !data || size == 0) {
        return false;
    }

    // Store sound data
    m_soundBuffers[name] = std::vector<char>(data, data + size);
    
    // Try to load actual WAV files for menu sounds
    std::cout << "Loading WAV file for: " << name << std::endl;
    
#ifdef __EMSCRIPTEN__
    // Try to load actual WAV files for menu sounds
    return EM_ASM_INT({
        try {
            const name = UTF8ToString($0);
            console.log('Loading WAV file for:', name);
            
            // Map menu sound paths to our organized structure
            let wavFileName = 'sounds/menu/CLICK.WAV'; // default
            
            // Check if this is a menu selection sound
            if (name.includes('SELECT.WAV') || name.includes('SELECT_MENU_ITEM')) {
                wavFileName = 'sounds/menu/SELECT.WAV';
            } else if (name.includes('CLICK.WAV') || name.includes('CHANGE_MENU_ITEM')) {
                wavFileName = 'sounds/menu/CLICK.WAV';

            }
            
            // Use fetch to load the WAV file
            console.log('Attempting to fetch:', wavFileName);
            fetch(wavFileName)
                .then(function(response) {
                    console.log('Fetch response status:', response.status, response.statusText);
                    if (!response.ok) {
                        throw new Error('Failed to load WAV file: ' + response.status + ' ' + response.statusText);
                    }
                    return response.arrayBuffer();
                })
                .then(function(arrayBuffer) {
                    const audioContext = window.audioContext;
                    return audioContext.decodeAudioData(arrayBuffer);
                })
                .then(function(audioBuffer) {
                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(name, audioBuffer);
                    console.log('Loaded WAV sound:', name, 'size:', audioBuffer.length, 'channels:', audioBuffer.numberOfChannels, 'sampleRate:', audioBuffer.sampleRate);
                })
                .catch(function(error) {
                    console.error('Error loading WAV file:', error);
                    // Fallback to oscillator if WAV loading fails
                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(name, {
                        type: 'oscillator',
                        frequency: 800,
                        duration: 0.5
                    });
                    console.log('Fallback to oscillator for:', name);
                });
            
            return true;
        } catch (e) {
            console.error('Error loading sound:', e);
            return false;
        }
    }, name.c_str());
#endif

    return true;

    std::cout << "Loaded sound: " << name << " (size: " << size << " bytes)" << std::endl;
    return true;
}

bool AudioWorkletSystem::PlaySound(const std::string& name, float volume) {
    if (!m_initialized || !m_soundEnabled || m_soundBuffers.find(name) == m_soundBuffers.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Play sound using Web Audio API
    return EM_ASM_INT({
        try {
            const name = UTF8ToString($0);
            const volume = $1;
            
            const soundBuffers = window.soundBuffers;
            if (!soundBuffers || !soundBuffers.has(name)) {
                console.log('Sound not loaded yet:', name, '- will retry later');
                return false;
            }
            
            const soundData = soundBuffers.get(name);
            const audioContext = window.audioContext;
            
            if (soundData.type === 'oscillator') {
                // Play oscillator-based test sound
                const oscillator = audioContext.createOscillator();
                const gainNode = audioContext.createGain();
                
                oscillator.frequency.setValueAtTime(soundData.frequency, audioContext.currentTime);
                oscillator.type = 'sine';
                
                gainNode.gain.setValueAtTime(volume * window.soundVolume, audioContext.currentTime);
                gainNode.gain.exponentialRampToValueAtTime(0.01, audioContext.currentTime + soundData.duration);
                
                oscillator.connect(gainNode);
                gainNode.connect(audioContext.destination);
                oscillator.start();
                oscillator.stop(audioContext.currentTime + soundData.duration);
                
                console.log('Playing test sound:', name, 'frequency:', soundData.frequency, 'volume:', volume);
                return true;
            } else {
                // Play buffer-based sound (original code)
                const source = audioContext.createBufferSource();
                const gainNode = audioContext.createGain();
                
                source.buffer = soundData;
                source.loop = false;
                gainNode.gain.value = volume * window.soundVolume;
                
                source.connect(gainNode);
                gainNode.connect(audioContext.destination);
                source.start();
                
                console.log('Playing buffer sound:', name, 'volume:', volume);
                return true;
            }
        } catch (e) {
            console.error('Error playing sound:', e);
            return false;
        }
    }, name.c_str(), volume * m_soundVolume);
#endif

    std::cout << "Playing sound: " << name << " (volume: " << volume << ")" << std::endl;
    return true;
}

void AudioWorkletSystem::StopSound(const std::string& name) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'stopSound',
                name: UTF8ToString($0)
            });
        }
    }, name.c_str());
#endif
}

void AudioWorkletSystem::StopAllSounds() {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'stopAllSounds'
            });
        }
    });
#endif
}

bool AudioWorkletSystem::LoadMusic(const std::string& name, const char* data, size_t size) {
    if (!m_initialized || !data || size == 0) {
        return false;
    }

    // Store music data
    m_musicBuffers[name] = std::vector<char>(data, data + size);
    
#ifdef __EMSCRIPTEN__
    // Send music data to AudioWorklet
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'loadMusic',
                name: UTF8ToString($0),
                data: new Uint8Array(HEAPU8.buffer, $1, $2)
            });
        }
    }, name.c_str(), data, size);
#endif

    std::cout << "Loaded music: " << name << " (size: " << size << " bytes)" << std::endl;
    return true;
}

bool AudioWorkletSystem::PlayMusic(const std::string& name, bool looping) {
    if (!m_initialized || !m_musicEnabled || m_musicBuffers.find(name) == m_musicBuffers.end()) {
        return false;
    }

    m_currentMusic = name;
    m_musicLooping = looping;
    m_musicPlaying = true;

#ifdef __EMSCRIPTEN__
    // Send play command to AudioWorklet
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'playMusic',
                name: UTF8ToString($0),
                looping: $1,
                volume: $2
            });
        }
    }, name.c_str(), looping, m_musicVolume);
#endif

    std::cout << "Playing music: " << name << " (looping: " << (looping ? "yes" : "no") << ")" << std::endl;
    return true;
}

void AudioWorkletSystem::StopMusic() {
    if (!m_musicPlaying) {
        return;
    }

    m_musicPlaying = false;
    m_currentMusic.clear();

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'stopMusic'
            });
        }
    });
#endif

    std::cout << "Music stopped" << std::endl;
}

void AudioWorkletSystem::PauseMusic() {
    if (!m_musicPlaying) {
        return;
    }

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'pauseMusic'
            });
        }
    });
#endif

    std::cout << "Music paused" << std::endl;
}

void AudioWorkletSystem::ResumeMusic() {
    if (!m_musicPlaying) {
        return;
    }

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'resumeMusic'
            });
        }
    });
#endif

    std::cout << "Music resumed" << std::endl;
}

bool AudioWorkletSystem::PlaySoundWithPath(const std::string& originalPath, const char* data, size_t size, float volume) {
    if (!m_initialized || !m_soundEnabled || !data || size == 0) {
        return false;
    }

    // Store sound data
    m_soundBuffers[originalPath] = std::vector<char>(data, data + size);
    
    std::cout << "Loading WAV file for: " << originalPath << std::endl;
    
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({
        try {
            const originalPath = UTF8ToString($0);
            const volume = $1;
            console.log('Loading WAV file for:', originalPath);
            
            // Map original paths to our organized structure
            let wavFileName = 'sounds/menu/CLICK.WAV'; // default
            
            if (originalPath.includes('SELECT.WAV') || originalPath.includes('SELECT_MENU_ITEM')) {
                wavFileName = 'sounds/menu/SELECT.WAV';
            } else if (originalPath.includes('CLICK.WAV') || originalPath.includes('CHANGE_MENU_ITEM')) {
                wavFileName = 'sounds/menu/CLICK.WAV';

            }
            
            // Use fetch to load the WAV file
            console.log('Attempting to fetch:', wavFileName);
            fetch(wavFileName)
                .then(function(response) {
                    console.log('Fetch response status:', response.status, response.statusText);
                    if (!response.ok) {
                        throw new Error('Failed to load WAV file: ' + response.status + ' ' + response.statusText);
                    }
                    return response.arrayBuffer();
                })
                .then(function(arrayBuffer) {
                    const audioContext = window.audioContext;
                    return audioContext.decodeAudioData(arrayBuffer);
                })
                .then(function(audioBuffer) {
                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(originalPath, audioBuffer);
                    console.log('Loaded WAV sound:', originalPath, 'size:', audioBuffer.length, 'channels:', audioBuffer.numberOfChannels, 'sampleRate:', audioBuffer.sampleRate);
                    
                    // Play the sound immediately after loading
                    const source = audioContext.createBufferSource();
                    const gainNode = audioContext.createGain();
                    
                    source.buffer = audioBuffer;
                    source.loop = false;
                    gainNode.gain.value = volume * window.soundVolume;
                    
                    source.connect(gainNode);
                    gainNode.connect(audioContext.destination);
                    source.start();
                    
                    console.log('Playing buffer sound:', originalPath, 'volume:', volume);
                })
                .catch(function(error) {
                    console.error('Error loading WAV file:', error);
                    // Fallback to oscillator if WAV loading fails
                    window.soundBuffers = window.soundBuffers || new Map();
                    window.soundBuffers.set(originalPath, {
                        type: 'oscillator',
                        frequency: 800,
                        duration: 0.5
                    });
                    console.log('Fallback to oscillator for:', originalPath);
                });
            
            return true;
        } catch (e) {
            console.error('Error loading sound:', e);
            return false;
        }
    }, originalPath.c_str(), volume * m_soundVolume);
#endif
    
    return true;
}

void AudioWorkletSystem::SetSoundVolume(float volume) {
    m_soundVolume = std::max(0.0f, std::min(1.0f, volume));
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        window.soundVolume = $0;
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'setSoundVolume',
                volume: $0
            });
        }
    }, m_soundVolume);
#endif
}

void AudioWorkletSystem::SetMusicVolume(float volume) {
    m_musicVolume = std::max(0.0f, std::min(1.0f, volume));
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        window.musicVolume = $0;
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage({
                type: 'setMusicVolume',
                volume: $0
            });
        }
    }, m_musicVolume);
#endif
}

#ifdef __EMSCRIPTEN__
bool AudioWorkletSystem::InitializeAudioWorklet() {
    return EM_ASM_INT({
        try {
            // Create AudioContext
            window.AudioContext = window.AudioContext || window.webkitAudioContext;
            if (!window.AudioContext) {
                console.error('Web Audio API not supported');
                return false;
            }
            
            window.audioContext = new AudioContext();
            
            // Initialize global volume variables
            window.soundVolume = 1.0;
            window.musicVolume = 1.0;
            
            // Resume audio context on user interaction
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
            
            console.log('AudioContext initialized for AudioWorklet');
            return true;
        } catch (e) {
            console.error('Error initializing AudioContext:', e);
            return false;
        }
    });
}

bool AudioWorkletSystem::LoadAudioWorkletScript() {
    return EM_ASM_INT({
        try {
            // Check if AudioWorklet is supported
            if (!window.audioContext || !window.audioContext.audioWorklet) {
                console.error('AudioWorklet not supported in this browser');
                return false;
            }
            
            // Create AudioWorklet script
            const audioWorkletScript = 
                'class GameAudioProcessor extends AudioWorkletProcessor {' +
                '    constructor() {' +
                '        super();' +
                '        this.soundVolume = 1.0;' +
                '        this.musicVolume = 1.0;' +
                '        this.port.onmessage = (event) => { this.handleMessage(event.data); };' +
                '    }' +
                '    handleMessage(data) {' +
                '        switch (data.type) {' +
                '            case "setSoundVolume": this.soundVolume = data.volume; break;' +
                '            case "setMusicVolume": this.musicVolume = data.volume; break;' +
                '        }' +
                '    }' +
                '    process(inputs, outputs, parameters) {' +
                '        // Simple audio processing - just pass through with volume control' +
                '        const output = outputs[0];' +
                '        if (output && output.length > 0) {' +
                '            for (let channel = 0; channel < output.length; channel++) {' +
                '                const outputChannel = output[channel];' +
                '                if (outputChannel) {' +
                '                    for (let i = 0; i < outputChannel.length; i++) {' +
                '                        outputChannel[i] = 0; // Clear output' +
                '                    }' +
                '                }' +
                '            }' +
                '        }' +
                '        return true;' +
                '    }' +
                '}' +
                'registerProcessor("game-audio-processor", GameAudioProcessor);';
            
            // Create blob and load AudioWorklet
            const blob = new Blob([audioWorkletScript], { type: 'application/javascript' });
            const url = URL.createObjectURL(blob);
            
            // Load AudioWorklet synchronously to avoid timing issues
            return new Promise((resolve, reject) => {
                window.audioContext.audioWorklet.addModule(url)
                    .then(() => {
                        try {
                            window.audioWorkletNode = new AudioWorkletNode(window.audioContext, 'game-audio-processor');
                            window.audioWorkletNode.connect(window.audioContext.destination);
                            console.log('AudioWorklet loaded successfully');
                            resolve(true);
                        } catch (e) {
                            console.error('Error creating AudioWorkletNode:', e);
                            reject(e);
                        }
                    })
                    .catch(error => {
                        console.error('Error loading AudioWorklet module:', error);
                        reject(error);
                    });
            }).then(() => true).catch(() => false);
        } catch (e) {
            console.error('Error setting up AudioWorklet:', e);
            return false;
        }
    });
}

void AudioWorkletSystem::SendMessageToAudioWorklet(const std::string& message) {
    EM_ASM({
        if (window.audioWorkletNode) {
            window.audioWorkletNode.port.postMessage(JSON.parse(UTF8ToString($0)));
        }
    }, message.c_str());
}
#else
bool AudioWorkletSystem::InitializeAudioWorklet() {
    return false;
}

bool AudioWorkletSystem::LoadAudioWorkletScript() {
    return false;
}

void AudioWorkletSystem::SendMessageToAudioWorklet(const std::string& message) {
    // No-op for non-WASM builds
}
#endif
