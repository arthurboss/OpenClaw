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
    
    // Try WebGPU first (priority)
    if (TryInitializeWebGPU()) {
        currentType = RendererType::WebGPU;
        LOG("WebGPU renderer initialized successfully");
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    // Fallback to WebGL2
    if (TryInitializeWebGL2()) {
        currentType = RendererType::WebGL2;
        LOG("WebGL2 renderer initialized (fallback)");
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    // Final fallback to WebGL1
    if (TryInitializeWebGL1()) {
        currentType = RendererType::WebGL1;
        LOG("WebGL1 renderer initialized (fallback)");
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
        try {
            return typeof navigator.gpu !== 'undefined' ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });
    
    if (webgpuAvailable) {
        LOG("WebGPU detected, attempting to initialize renderer");
        try {
            renderer = std::make_unique<WebGPURenderer>();
            if (renderer->Initialize()) {
                LOG("WebGPU renderer initialized successfully");
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
        LOG("WebGPU not available");
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
            renderer = std::make_unique<WebGLRenderer>();
            if (renderer->Initialize()) {
                LOG("WebGL2 renderer initialized successfully");
                return true;
            } else {
                LOG("WebGL2 renderer initialization failed");
                renderer.reset();
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGL2 initialization: " + std::string(e.what()));
            renderer.reset();
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
            renderer = std::make_unique<WebGLRenderer>();
            if (renderer->Initialize()) {
                LOG("WebGL1 renderer initialized successfully");
                return true;
            } else {
                LOG("WebGL1 renderer initialization failed");
                renderer.reset();
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGL1 initialization: " + std::string(e.what()));
            renderer.reset();
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
