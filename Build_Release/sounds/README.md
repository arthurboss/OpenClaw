# Audio File Structure

## Overview
This directory contains the organized audio assets for the OpenClaw project. The audio system supports multiple platforms including WebAssembly (WASM), native builds, and other target platforms.

## Directory Structure
```
sounds/
├── README.md               # This documentation
├── AUDIO_FILES.md          # Complete audio files reference
├── menu/                   # Menu and UI interaction sounds
├── gameplay/              # In-game sound effects (future)
└── music/                 # Background music files (future)
```

> **📋 Reference**: See [AUDIO_FILES.md](AUDIO_FILES.md) for a complete list of all available audio files with sizes and purposes.

## Audio System Architecture

### Platform Support
- **WASM Builds**: Uses Web Audio API with fetch-based lazy loading
- **Native Builds**: Uses SDL2_mixer for direct file access
- **Other Platforms**: Extensible through the `IAudioSystem` interface

### Loading Strategy
- **Lazy Loading**: Audio files are loaded on-demand for optimal performance
- **Caching**: Loaded audio is cached in memory for repeated playback
- **Fallback**: Graceful degradation when audio files are unavailable

## File Format Requirements

### Supported Formats
- **WAV**: Primary format for all audio assets
- **Sample Rate**: 44.1kHz recommended
- **Channels**: Mono or Stereo
- **Bit Depth**: 16-bit recommended

### File Naming Convention
- **UPPERCASE**: All audio files use uppercase naming
- **Descriptive**: Names should clearly indicate the sound's purpose
- **Consistent**: Follow established patterns (e.g., `CLICK.WAV`, `SELECT.WAV`)

## Development Guidelines

### Adding New Audio Files
1. **Place in appropriate directory** based on usage category
2. **Use consistent naming** following established conventions
3. **Update documentation** to reflect new audio assets
4. **Test across platforms** to ensure compatibility

### Audio Quality Standards
- **File Size**: Keep files as small as possible while maintaining quality
- **Duration**: Menu sounds should be brief (0.1-0.5 seconds)
- **Volume**: Normalize audio levels for consistent playback
- **Format**: Use uncompressed WAV for maximum compatibility

### Platform-Specific Considerations

#### WASM Builds
- **Fetch Loading**: Files are loaded via HTTP requests
- **Caching**: Browser caching improves performance
- **Compression**: Consider gzip compression for web delivery

#### Native Builds
- **Direct Access**: Files are loaded directly from filesystem
- **SDL2_mixer**: Uses SDL2_mixer for audio playback
- **Resource Management**: Automatic cleanup on application exit

## Integration

### Audio System Interface
The audio system uses a modular architecture with platform-specific implementations:

```cpp
// Abstract interface for all audio systems
class IAudioSystem {
    virtual bool LoadSound(const std::string& name, const char* data, size_t size) = 0;
    virtual bool PlaySound(const std::string& name, float volume) = 0;
    // ... other methods
};
```

### Usage in Code
```cpp
// Load and play menu sounds
audioSystem->LoadSound("CLICK.WAV", data, size);
audioSystem->PlaySound("CLICK.WAV", 1.0f);
```

## Future Expansion

### Planned Categories
- **Gameplay Sounds**: Combat, movement, and interaction sounds
- **Music System**: Background music with proper looping
- **Ambient Audio**: Environmental and atmospheric sounds
- **Voice Lines**: Character dialogue and responses

### Performance Optimizations
- **Audio Streaming**: For longer audio files like music
- **Spatial Audio**: 3D positioning for immersive gameplay
- **Dynamic Loading**: Level-specific audio loading
- **Compression**: Adaptive compression based on platform capabilities

## Troubleshooting

### Common Issues
1. **File Not Found**: Ensure audio files are in the correct directory structure
2. **Loading Failures**: Check file format and naming conventions
3. **Playback Issues**: Verify audio system initialization
4. **Platform Differences**: Test on all target platforms

### Debug Information
- **WASM**: Check browser console for fetch errors
- **Native**: Verify SDL2_mixer initialization
- **Logs**: Audio system provides detailed logging for debugging

## Contributing
When adding new audio assets:
1. Follow the established directory structure
2. Use appropriate file formats and naming conventions
3. Test on all supported platforms
4. Update this documentation
5. Ensure audio quality meets project standards

## License
Audio assets are subject to the same license as the OpenClaw project. Ensure you have proper rights to use any audio files included in this project.
