#pragma once

#include "IRenderer.h"
#include <SDL2/SDL.h>
#include <memory>
#include <string>

// Forward declarations
class WebGPURenderer;
class WebGLRenderer;

// Renderer type enumeration
enum class RendererType {
    WebGPU,
    WebGL2,
    WebGL1,
    None
};

// Graphics manager class
class GraphicsManager {
private:
    std::unique_ptr<IRenderer> renderer;
    RendererType currentType;
    bool isInitialized;
    SDL_Renderer* existingSdlRenderer; // Store existing SDL renderer for WebGL fallback
    
    // Performance tracking
    float frameTime;
    int drawCalls;
    
public:
    
    // Constructor/Destructor
    GraphicsManager();
    ~GraphicsManager();
    
    // Initialization and shutdown
    bool Initialize();
    bool Initialize(SDL_Renderer* existingRenderer); // Overload to use existing renderer
    void Shutdown();
    
    // Renderer access
    IRenderer* GetRenderer() { return renderer.get(); }
    const IRenderer* GetRenderer() const { return renderer.get(); }
    RendererType GetCurrentType() const { return currentType; }
    
    // Status queries
    bool IsInitialized() const { return isInitialized; }
    std::string GetRendererName() const;
    bool SupportsFeature(RendererFeature feature) const;
    
    // Renderer status and information
    std::string GetRendererStatus() const;
    bool IsUsingWebGPU() const { return currentType == RendererType::WebGPU; }
    bool IsUsingWebGL() const { return currentType == RendererType::WebGL2 || currentType == RendererType::WebGL1; }
    
    // Performance queries
    float GetFrameTime() const { return frameTime; }
    int GetDrawCalls() const { return drawCalls; }
    void ResetStats();
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    
private:
    // Renderer detection
    bool InitializeInternal(SDL_Renderer* existingRenderer); // Common initialization logic
    bool TryInitializeWebGPU();
    bool TryInitializeWebGL2();
    bool TryInitializeWebGL1();
    
    // Helper methods
    void LogRendererInfo() const;
};
