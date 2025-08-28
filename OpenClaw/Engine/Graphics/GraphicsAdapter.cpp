#include "GraphicsAdapter.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>

// Constructor
GraphicsAdapter::GraphicsAdapter()
    : m_isInitialized(false)
{
}

// Destructor
GraphicsAdapter::~GraphicsAdapter() {
    Shutdown();
}

// Initialize graphics adapter
bool GraphicsAdapter::Initialize() {
    LOG("Initializing GraphicsAdapter...");
    
    m_graphicsManager = std::make_unique<GraphicsManager>();
    if (!m_graphicsManager->Initialize()) {
        LOG_ERROR("Failed to initialize GraphicsManager");
        return false;
    }
    
    m_isInitialized = true;
    LOG("GraphicsAdapter initialized successfully");
    LOG("Active renderer: " + m_graphicsManager->GetRendererName());
    
    return true;
}

// Shutdown graphics adapter
void GraphicsAdapter::Shutdown() {
    if (m_graphicsManager) {
        m_graphicsManager->Shutdown();
        m_graphicsManager.reset();
    }
    
    m_isInitialized = false;
    LOG("GraphicsAdapter shutdown complete");
}

// Begin frame
void GraphicsAdapter::BeginFrame() {
    if (m_graphicsManager) {
        m_graphicsManager->BeginFrame();
    }
}

// End frame
void GraphicsAdapter::EndFrame() {
    if (m_graphicsManager) {
        m_graphicsManager->EndFrame();
    }
}

// Render background using new graphics system
void GraphicsAdapter::RenderBackground(shared_ptr<Image> background, const SDL_Rect& rect) {
    if (!m_isInitialized || !background) return;
    
    IRenderer* renderer = m_graphicsManager->GetRenderer();
    if (!renderer) return;
    
    // Create background data
    MenuBackgroundData backgroundData;
    backgroundData.texturePath = "background"; // Placeholder - in real implementation you'd store the actual path
    backgroundData.x = static_cast<float>(rect.x);
    backgroundData.y = static_cast<float>(rect.y);
    backgroundData.width = static_cast<float>(rect.w);
    backgroundData.height = static_cast<float>(rect.h);
    backgroundData.alpha = 1.0f;
    backgroundData.visible = true;
    
    // Render using new system
    renderer->RenderMenuBackground(backgroundData);
}

// Render menu item using new graphics system
void GraphicsAdapter::RenderMenuItem(const std::string& name, const Point& position, 
                                   float width, float height, ::MenuItemState state, bool visible) {
    if (!m_isInitialized) return;
    
    IRenderer* renderer = m_graphicsManager->GetRenderer();
    if (!renderer) return;
    
    // Create menu item data
    MenuItemData itemData;
    itemData.name = name;
    itemData.x = static_cast<float>(position.x);
    itemData.y = static_cast<float>(position.y);
    itemData.width = width;
    itemData.height = height;
    itemData.alpha = 1.0f;
    itemData.visible = visible;
    itemData.state = ConvertMenuItemState(state);
    itemData.canBeFocused = true;
    
    // Set texture paths (simplified - in real implementation you'd need to store these)
    itemData.inactiveTexturePath = "button_inactive";
    itemData.activeTexturePath = "button_active";
    
    // Render using new system
    renderer->RenderMenuItem(itemData);
}

// Render text using new graphics system
void GraphicsAdapter::RenderText(const std::string& text, const Point& position, float fontSize) {
    if (!m_isInitialized) return;
    
    IRenderer* renderer = m_graphicsManager->GetRenderer();
    if (!renderer) return;
    
    // Create text data
    MenuTextData textData;
    textData.text = text;
    textData.fontPath = "clacon.ttf";
    textData.fontSize = fontSize;
    textData.x = static_cast<float>(position.x);
    textData.y = static_cast<float>(position.y);
    textData.r = 1.0f;
    textData.g = 1.0f;
    textData.b = 1.0f;
    textData.a = 1.0f;
    textData.alignment = TextAlignment::Left;
    textData.visible = true;
    textData.bold = false;
    textData.italic = false;
    
    // Render using new system
    renderer->RenderMenuText(textData);
}

// Convert old MenuItemState to new MenuItemState
MenuItemState GraphicsAdapter::ConvertMenuItemState(::MenuItemState oldState) {
    switch (oldState) {
        case MenuItemState_Disabled:
            return MenuItemState::Inactive;
        case MenuItemState_Inactive:
            return MenuItemState::Inactive;
        case MenuItemState_Active:
            return MenuItemState::Active;
        case MenuItemState_None:
        default:
            return MenuItemState::Inactive;
    }
}

// Get renderer name
std::string GraphicsAdapter::GetRendererName() const {
    if (m_graphicsManager) {
        return m_graphicsManager->GetRendererName();
    }
    return "None";
}

// Get frame time
float GraphicsAdapter::GetFrameTime() const {
    if (m_graphicsManager) {
        return m_graphicsManager->GetFrameTime();
    }
    return 0.0f;
}

// Get draw calls
int GraphicsAdapter::GetDrawCalls() const {
    if (m_graphicsManager) {
        return m_graphicsManager->GetDrawCalls();
    }
    return 0;
}

// Reset stats
void GraphicsAdapter::ResetStats() {
    if (m_graphicsManager) {
        m_graphicsManager->ResetStats();
    }
}
