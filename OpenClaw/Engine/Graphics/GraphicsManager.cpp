#include "GraphicsManager.h"
#include "WebGL/WebGLRenderer.h"
#include "WebGPU/WebGPURenderer.h"
#include "../Logger/Logger.h"
#include <iostream>
#include <memory>
#include <emscripten.h>
#include <stdexcept>

// Constructor
GraphicsManager::GraphicsManager()
    : currentType(RendererType::None)
    , isInitialized(false)
    , existingSdlRenderer(nullptr)
    , frameTime(0.0f)
    , drawCalls(0)
{
    LOG("GraphicsManager constructor called");
}

// Destructor
GraphicsManager::~GraphicsManager() {
    LOG("GraphicsManager destructor called");
    Shutdown();
}

// Initialize graphics system
bool GraphicsManager::Initialize() {
    LOG("=== GraphicsManager::Initialize() called ===");
    
    return InitializeInternal(nullptr);
}

// Initialize with existing SDL renderer
bool GraphicsManager::Initialize(SDL_Renderer* existingRenderer) {
    LOG("=== GraphicsManager::Initialize() called with existing renderer ===");
    
    return InitializeInternal(existingRenderer);
}

// Common initialization logic
bool GraphicsManager::InitializeInternal(SDL_Renderer* existingRenderer) {
    if (isInitialized) {
        LOG_WARNING("GraphicsManager already initialized");
        return true;
    }
    
    // Store the existing renderer for WebGL fallback
    existingSdlRenderer = existingRenderer;
    
    // Try to initialize renderers in order of preference
    if (TryInitializeWebGPU()) {
        currentType = RendererType::WebGPU;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    if (TryInitializeWebGL2()) {
        currentType = RendererType::WebGL2;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    if (TryInitializeWebGL1()) {
        currentType = RendererType::WebGL1;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    LOG_ERROR("No graphics renderer available");
    return false;
}

// Shutdown graphics system
void GraphicsManager::Shutdown() {
    LOG("GraphicsManager::Shutdown() called");
    if (renderer) {
        renderer->Shutdown();
        renderer.reset();
    }
    
    currentType = RendererType::None;
    isInitialized = false;
    
    LOG("Graphics Manager shutdown complete");
}

// Get renderer name
std::string GraphicsManager::GetRendererName() const {
    if (renderer) {
        return renderer->GetRendererName();
    }
    return "None";
}

// Get detailed renderer status
std::string GraphicsManager::GetRendererStatus() const {
    std::string status = "Graphics System Status:\n";
    status += "  Initialized: " + std::string(isInitialized ? "Yes" : "No") + "\n";
    status += "  Renderer Type: ";
    
    switch (currentType) {
        case RendererType::WebGPU:
            status += "WebGPU (Modern, High Performance)";
            break;
        case RendererType::WebGL2:
            status += "WebGL2 (Fallback, Good Performance)";
            break;
        case RendererType::WebGL1:
            status += "WebGL1 (Legacy Fallback)";
            break;
        case RendererType::None:
            status += "None (No Graphics Available)";
            break;
    }
    
    status += "\n  Renderer Name: " + GetRendererName() + "\n";
    
    if (renderer) {
        status += "  WebGPU Support: " + std::string(SupportsFeature(RendererFeature::WebGPU) ? "Yes" : "No") + "\n";
        status += "  WebGL2 Support: " + std::string(SupportsFeature(RendererFeature::WebGL2) ? "Yes" : "No") + "\n";
        status += "  WebGL1 Support: " + std::string(SupportsFeature(RendererFeature::WebGL1) ? "Yes" : "No") + "\n";
        status += "  Shader Support: " + std::string(SupportsFeature(RendererFeature::ShaderSupport) ? "Yes" : "No") + "\n";
        status += "  Texture Compression: " + std::string(SupportsFeature(RendererFeature::TextureCompression) ? "Yes" : "No") + "\n";
    }
    
    return status;
}

// Check feature support
bool GraphicsManager::SupportsFeature(RendererFeature feature) const {
    if (renderer) {
        return renderer->SupportsFeature(feature);
    }
    return false;
}

// Reset performance stats
void GraphicsManager::ResetStats() {
    frameTime = 0.0f;
    drawCalls = 0;
    
    if (renderer) {
        renderer->ResetStats();
    }
}

// Begin frame
void GraphicsManager::BeginFrame() {
    if (renderer && isInitialized) {
        renderer->BeginFrame();
    }
}

// End frame
void GraphicsManager::EndFrame() {
    if (renderer && isInitialized) {
        renderer->EndFrame();
        
        // Update performance stats
        frameTime = renderer->GetFrameTime();
        drawCalls = renderer->GetDrawCalls();
    }
}

// Try to initialize WebGPU
bool GraphicsManager::TryInitializeWebGPU() {
    LOG("GraphicsManager::TryInitializeWebGPU() called");
    
    // Check if WebGPU is available via JavaScript
    int webgpuAvailable = EM_ASM_INT({
        return Module.detectWebGPU();
    });
    
    if (webgpuAvailable) {
        LOG("WebGPU detected, attempting to initialize renderer");
        
        // Get detailed WebGPU info from JavaScript
        char* webgpuInfo = (char*)EM_ASM_PTR({
            var info = Module.getWebGPUInfo();
            var length = lengthBytesUTF8(info) + 1;
            var buffer = _malloc(length);
            stringToUTF8(info, buffer, length);
            return buffer;
        });
        
        LOG("WebGPU Info: " + std::string(webgpuInfo));
        free(webgpuInfo);
        
        try {
            renderer.reset(new WebGPURenderer());
            if (renderer->Initialize()) {
                LOG("WebGPU renderer initialized successfully");
                LOG("🎉 WebGPU is now active! Better performance expected.");
                return true;
            } else {
                LOG("WebGPU renderer initialization failed");
                renderer.reset();
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGPU initialization: " + std::string(e.what()));
            renderer.reset();
        }
    } else {
        LOG("WebGPU not available in this browser");
    }
    
    return false;
}

// Try to initialize WebGL2
bool GraphicsManager::TryInitializeWebGL2() {
    LOG("GraphicsManager::TryInitializeWebGL2() called");
    
    // Check if WebGL2 is available via JavaScript
    int webgl2Available = EM_ASM_INT({
        try {
            var canvas = document.createElement('canvas');
            var gl = canvas.getContext('webgl2');
            return gl ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });
    
    if (webgl2Available) {
        LOG("WebGL2 detected, attempting to initialize renderer");
        try {
            WebGLRenderer* webglRenderer = new WebGLRenderer();
            bool initSuccess = false;
            
            if (existingSdlRenderer) {
                LOG("Using existing SDL renderer for WebGL2");
                initSuccess = webglRenderer->Initialize(existingSdlRenderer);
            } else {
                LOG("Creating new SDL renderer for WebGL2");
                initSuccess = webglRenderer->Initialize();
            }
            
            if (initSuccess) {
                renderer.reset(webglRenderer);
                LOG("WebGL2 renderer initialized successfully");
                return true;
            } else {
                LOG("WebGL2 renderer initialization failed");
                delete webglRenderer;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGL2 initialization: " + std::string(e.what()));
        }
    } else {
        LOG("WebGL2 not available");
    }
    
    return false;
}

// Try to initialize WebGL1
bool GraphicsManager::TryInitializeWebGL1() {
    LOG("GraphicsManager::TryInitializeWebGL1() called");
    
    // Check if WebGL1 is available via JavaScript
    int webgl1Available = EM_ASM_INT({
        try {
            var canvas = document.createElement('canvas');
            var gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
            return gl ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });
    
    if (webgl1Available) {
        LOG("WebGL1 detected, attempting to initialize renderer");
        try {
            WebGLRenderer* webglRenderer = new WebGLRenderer();
            bool initSuccess = false;
            
            if (existingSdlRenderer) {
                LOG("Using existing SDL renderer for WebGL1");
                initSuccess = webglRenderer->Initialize(existingSdlRenderer);
            } else {
                LOG("Creating new SDL renderer for WebGL1");
                initSuccess = webglRenderer->Initialize();
            }
            
            if (initSuccess) {
                renderer.reset(webglRenderer);
                LOG("WebGL1 renderer initialized successfully");
                return true;
            } else {
                LOG("WebGL1 renderer initialization failed");
                delete webglRenderer;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGL1 initialization: " + std::string(e.what()));
        }
    } else {
        LOG("WebGL1 not available");
    }
    
    return false;
}

// Log renderer information
void GraphicsManager::LogRendererInfo() const {
    LOG("GraphicsManager::LogRendererInfo() called");
    if (renderer) {
        LOG("Active Renderer: " + renderer->GetRendererName());
        LOG("WebGPU Support: " + std::string(SupportsFeature(RendererFeature::WebGPU) ? "Yes" : "No"));
        LOG("WebGL2 Support: " + std::string(SupportsFeature(RendererFeature::WebGL2) ? "Yes" : "No"));
        LOG("WebGL1 Support: " + std::string(SupportsFeature(RendererFeature::WebGL1) ? "Yes" : "No"));
    } else {
        LOG("No renderer available");
    }
}
