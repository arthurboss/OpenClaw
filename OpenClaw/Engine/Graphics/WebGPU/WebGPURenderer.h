#pragma once

#include "../IRenderer.h"
#include "../Data/MenuBackgroundData.h"
#include "../Data/MenuItemData.h"
#include "../Data/MenuTextData.h"
#include <webgpu/webgpu.h>
#include <map>
#include <string>
#include <vector>

// WebGPU renderer implementation
class WebGPURenderer : public IRenderer {
private:
    // WebGPU objects
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSwapChain swapChain;
    WGPURenderPipeline renderPipeline;
    
    // Surface and configuration
    WGPUSurface surface;
    WGPUSurfaceConfiguration surfaceConfig;
    int surfaceWidth, surfaceHeight;
    
    // State
    bool isInitialized;
    
    // Performance tracking
    float frameTime;
    int drawCalls;
    uint32_t frameStartTime;
    
    // Viewport
    int viewportX, viewportY, viewportWidth, viewportHeight;
    
    // Texture cache
    std::map<std::string, WGPUTextureView> textureCache;
    
    // Shader modules
    WGPUShaderModule vertexShader;
    WGPUShaderModule fragmentShader;
    
public:
    // Constructor/Destructor
    WebGPURenderer();
    ~WebGPURenderer();
    
    // Core rendering operations
    bool Initialize() override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    
    // Menu-specific rendering
    void RenderMenuBackground(const MenuBackgroundData& data) override;
    void RenderMenuItem(const MenuItemData& data) override;
    void RenderMenuText(const MenuTextData& data) override;
    
    // Common operations
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;
    void Present() override;
    
    // Capability queries
    bool SupportsFeature(RendererFeature feature) override;
    std::string GetRendererName() const override;
    
    // Performance queries
    float GetFrameTime() const override { return frameTime; }
    int GetDrawCalls() const override { return drawCalls; }
    void ResetStats() override;
    
private:
    // Helper methods
    void ResetPerformanceStats();
    bool InitializeWebGPU();
    bool CreateSwapChain();
    bool CreateRenderPipeline();
    bool CreateShaders();
    WGPUTextureView LoadTexture(const std::string& path);
    void RenderTexture(WGPUTextureView texture, float x, float y, float width, float height, float alpha);
    void RenderTexture(const std::string& texturePath, float x, float y, float width, float height, float alpha);
    void ClearTextureCache();
    WGPUTextureView CreateTextTexture(const std::string& text, const MenuTextData& textData);
    
    // WebGPU callbacks
    static void OnDeviceLost(WGPUDeviceLostReason reason, const char* message, void* userdata);
    static void OnError(WGPUErrorType type, const char* message, void* userdata);
};
