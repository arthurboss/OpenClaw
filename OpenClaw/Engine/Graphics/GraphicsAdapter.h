#pragma once

#include "GraphicsManager.h"
#include "Data/MenuBackgroundData.h"
#include "Data/MenuItemData.h"
#include "Data/MenuTextData.h"
#include <SDL2/SDL.h>
#include <memory>

// Forward declarations
class Image;

// Graphics adapter that provides a bridge to the new modular graphics system
class GraphicsAdapter {
private:
    std::unique_ptr<GraphicsManager> m_graphicsManager;
    bool m_isInitialized;
    
public:
    GraphicsAdapter();
    ~GraphicsAdapter();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Direct rendering methods (for integration with existing system)
    void BeginFrame();
    void EndFrame();
    
    // Background rendering
    void RenderBackground(shared_ptr<Image> background, const SDL_Rect& rect);
    
    // Menu item rendering
    void RenderMenuItem(const std::string& name, const Point& position, 
                       float width, float height, ::MenuItemState state, bool visible);
    
    // Text rendering
    void RenderText(const std::string& text, const Point& position, float fontSize = 16.0f);
    
    // Utility methods
    bool IsInitialized() const { return m_isInitialized; }
    GraphicsManager* GetGraphicsManager() { return m_graphicsManager.get(); }
    std::string GetRendererName() const;
    
    // Performance queries
    float GetFrameTime() const;
    int GetDrawCalls() const;
    void ResetStats();
    
private:
    // Helper methods
    MenuItemState ConvertMenuItemState(::MenuItemState oldState);
};
