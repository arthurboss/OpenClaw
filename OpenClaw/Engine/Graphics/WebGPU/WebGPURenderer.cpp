#include "WebGPURenderer.h"
#include "WebGPUShaders.h"
#include "../../Logger/Logger.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>

// Constructor
WebGPURenderer::WebGPURenderer()
    : instance(nullptr)
    , adapter(nullptr)
    , device(nullptr)
    , queue(nullptr)
    , swapChain(nullptr)
    , renderPipeline(nullptr)
    , surface(nullptr)
    , isInitialized(false)
    , frameTime(0.0f)
    , drawCalls(0)
    , frameStartTime(0)
    , viewportX(0), viewportY(0), viewportWidth(1280), viewportHeight(960)
    , surfaceWidth(1280), surfaceHeight(960)
    , vertexShader(nullptr)
    , fragmentShader(nullptr)
{
}

// Destructor
WebGPURenderer::~WebGPURenderer() {
    Shutdown();
}

// Initialize WebGPU renderer
bool WebGPURenderer::Initialize() {
    LOG("Initializing WebGPU Renderer...");
    
    if (!InitializeWebGPU()) {
        LOG_ERROR("Failed to initialize WebGPU");
        return false;
    }
    
    if (!CreateSwapChain()) {
        LOG_ERROR("Failed to create swap chain");
        return false;
    }
    
    if (!CreateShaders()) {
        LOG_ERROR("Failed to create shaders");
        return false;
    }
    
    if (!CreateRenderPipeline()) {
        LOG_ERROR("Failed to create render pipeline");
        return false;
    }
    
    isInitialized = true;
    ResetPerformanceStats();
    
    LOG("WebGPU Renderer initialized successfully");
    return true;
}

// Shutdown WebGPU renderer
void WebGPURenderer::Shutdown() {
    ClearTextureCache();
    
    if (fragmentShader) {
        wgpuShaderModuleRelease(fragmentShader);
        fragmentShader = nullptr;
    }
    
    if (vertexShader) {
        wgpuShaderModuleRelease(vertexShader);
        vertexShader = nullptr;
    }
    
    if (renderPipeline) {
        wgpuRenderPipelineRelease(renderPipeline);
        renderPipeline = nullptr;
    }
    
    if (swapChain) {
        wgpuSwapChainRelease(swapChain);
        swapChain = nullptr;
    }
    
    if (surface) {
        wgpuSurfaceRelease(surface);
        surface = nullptr;
    }
    
    if (queue) {
        wgpuQueueRelease(queue);
        queue = nullptr;
    }
    
    if (device) {
        wgpuDeviceRelease(device);
        device = nullptr;
    }
    
    if (adapter) {
        wgpuAdapterRelease(adapter);
        adapter = nullptr;
    }
    
    if (instance) {
        wgpuInstanceRelease(instance);
        instance = nullptr;
    }
    
    isInitialized = false;
    
    LOG("WebGPU Renderer shutdown complete");
}

// Begin frame
void WebGPURenderer::BeginFrame() {
    if (!isInitialized) return;
    
    frameStartTime = emscripten_get_now();
    drawCalls = 0;
    
    // Clear the screen
    Clear(0.0f, 0.0f, 0.0f, 1.0f);
}

// End frame
void WebGPURenderer::EndFrame() {
    if (!isInitialized) return;
    
    // Present the frame
    Present();
    
    // Calculate frame time
    float frameEndTime = emscripten_get_now();
    frameTime = frameEndTime - frameStartTime;
}

// Render menu background
void WebGPURenderer::RenderMenuBackground(const MenuBackgroundData& data) {
    if (!isInitialized || !data.visible) return;
    
    RenderTexture(data.texturePath, data.x, data.y, data.width, data.height, data.alpha);
    drawCalls++;
}

// Render menu item
void WebGPURenderer::RenderMenuItem(const MenuItemData& data) {
    if (!isInitialized || !data.visible) return;
    
    // Choose texture based on state
    std::string texturePath = (data.state == MenuItemState::Active) ? 
        data.activeTexturePath : data.inactiveTexturePath;
    
    RenderTexture(texturePath, data.x, data.y, data.width, data.height, data.alpha);
    drawCalls++;
}

// Render menu text
void WebGPURenderer::RenderMenuText(const MenuTextData& data) {
    if (!isInitialized || !data.visible) return;
    
    WGPUTextureView textTexture = CreateTextTexture(data.text, data);
    if (textTexture) {
        RenderTexture(textTexture, data.x, data.y, 0, 0, data.a);
        wgpuTextureViewRelease(textTexture);
        drawCalls++;
    }
}

// Set viewport
void WebGPURenderer::SetViewport(int x, int y, int width, int height) {
    viewportX = x;
    viewportY = y;
    viewportWidth = width;
    viewportHeight = height;
}

// Clear screen
void WebGPURenderer::Clear(float r, float g, float b, float a) {
    if (!isInitialized) return;
    
    // TODO: Implement WebGPU clear
    LOG("WebGPU Clear: (" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(a) + ")");
}

// Present frame
void WebGPURenderer::Present() {
    if (!isInitialized) return;
    
    // TODO: Implement WebGPU present
    LOG("WebGPU Present");
}

// Check feature support
bool WebGPURenderer::SupportsFeature(RendererFeature feature) {
    switch (feature) {
        case RendererFeature::WebGPU:
        case RendererFeature::TextureCompression:
        case RendererFeature::ShaderSupport:
        case RendererFeature::MultiSampling:
            return true;
        case RendererFeature::WebGL2:
        case RendererFeature::WebGL1:
            return false;
        default:
            return false;
    }
}

// Get renderer name
std::string WebGPURenderer::GetRendererName() const {
    return "WebGPU";
}

// Reset performance stats
void WebGPURenderer::ResetStats() {
    ResetPerformanceStats();
}

// Reset performance statistics
void WebGPURenderer::ResetPerformanceStats() {
    frameTime = 0.0f;
    drawCalls = 0;
    frameStartTime = 0;
}

// Initialize WebGPU
bool WebGPURenderer::InitializeWebGPU() {
    // Check if WebGPU is available via JavaScript
    int webgpuAvailable = EM_ASM_INT({
        return Module.detectWebGPU();
    });
    
    if (!webgpuAvailable) {
        LOG_ERROR("WebGPU not available in this browser");
        return false;
    }
    
    // Get WebGPU info from JavaScript
    char* webgpuInfo = (char*)EM_ASM_PTR({
        var info = Module.getWebGPUInfo();
        var length = lengthBytesUTF8(info) + 1;
        var buffer = _malloc(length);
        stringToUTF8(info, buffer, length);
        return buffer;
    });
    
    LOG("WebGPU Info: " + std::string(webgpuInfo));
    free(webgpuInfo);
    
    // Create WebGPU instance
    WGPUInstanceDescriptor instanceDesc = {};
    instance = wgpuCreateInstance(&instanceDesc);
    if (!instance) {
        LOG_ERROR("Failed to create WebGPU instance");
        return false;
    }
    
    // Request adapter
    WGPURequestAdapterOptions adapterOptions = {};
    adapterOptions.nextInChain = nullptr;
    adapterOptions.compatibleSurface = nullptr;
    
    // TODO: Implement proper adapter request
    LOG("WebGPU adapter request not yet implemented");
    return false;
}

// Create swap chain
bool WebGPURenderer::CreateSwapChain() {
    // TODO: Implement swap chain creation
    LOG("WebGPU swap chain creation not yet implemented");
    return false;
}

// Create render pipeline
bool WebGPURenderer::CreateRenderPipeline() {
    // TODO: Implement render pipeline creation
    LOG("WebGPU render pipeline creation not yet implemented");
    return false;
}

// Create shaders
bool WebGPURenderer::CreateShaders() {
    // TODO: Implement shader creation
    LOG("WebGPU shader creation not yet implemented");
    return false;
}

// Load texture with caching
WGPUTextureView WebGPURenderer::LoadTexture(const std::string& path) {
    // TODO: Implement WebGPU texture loading
    LOG("Loading WebGPU texture: " + path);
    return nullptr;
}

// Render texture with position and size
void WebGPURenderer::RenderTexture(WGPUTextureView texture, float x, float y, float width, float height, float alpha) {
    // TODO: Implement WebGPU texture rendering
    LOG("Rendering WebGPU texture at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

// Render texture by path
void WebGPURenderer::RenderTexture(const std::string& texturePath, float x, float y, float width, float height, float alpha) {
    WGPUTextureView texture = LoadTexture(texturePath);
    if (texture) {
        RenderTexture(texture, x, y, width, height, alpha);
    }
}

// Clear texture cache
void WebGPURenderer::ClearTextureCache() {
    for (auto& pair : textureCache) {
        if (pair.second) {
            wgpuTextureViewRelease(pair.second);
        }
    }
    textureCache.clear();
}

// Create text texture
WGPUTextureView WebGPURenderer::CreateTextTexture(const std::string& text, const MenuTextData& textData) {
    // TODO: Implement WebGPU text texture creation
    LOG("Creating WebGPU text texture: " + text);
    return nullptr;
}

// WebGPU callbacks
void WebGPURenderer::OnDeviceLost(WGPUDeviceLostReason reason, const char* message, void* userdata) {
    LOG_ERROR("WebGPU device lost: " + std::string(message));
}

void WebGPURenderer::OnError(WGPUErrorType type, const char* message, void* userdata) {
    LOG_ERROR("WebGPU error: " + std::string(message));
}
