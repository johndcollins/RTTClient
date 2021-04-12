#pragma once

#include <string>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

class CDisplayWindow
{
public:
    CDisplayWindow(const string& title, int x, int y, int width, int height, bool ontop);
    virtual ~CDisplayWindow();

    void SetBackgroundImage(string filename);

    void Render(bool connected, std::vector<unsigned char> image);
    
    bool HandleEvents(SDL_Event& event);

    void ShowWindow();
    void MoveWindow(int x, int y, int width, int height, bool ontop);
    void CloseWindow();

    inline bool IsClosed() const { return !m_bWindowShown; }
    inline bool IsCreated() const { return m_bWindow_Created; }

    inline void SetFlipImageVertically(bool set) { m_bFlipImageVertically = set; }
    inline void SetFlipImageHorizontally(bool set) { m_bFlipImageHorizontally = set; }

    inline void SetUseDefaultWidth(bool set) { m_bDefaultSize_W = set; }
    inline void SetUseDefaultHeight(bool set) { m_bDefaultSize_H = set; }

    inline void SetShowPositionInfo(bool set) { m_bShowPositionInfo = set; }

    void SetWindowMovable(bool set);

private:
    bool Init();

    SDL_Texture* LoadTexture(string path);
    void RenderText(string text, SDL_Rect dest, SDL_Color textColor);

    void ShowPositionText();

    void DrawDefaultBackground();

    static SDL_HitTestResult DraggingCallback(SDL_Window* win, const SDL_Point* area, void* data);

    SDL_Window*     m_pWindow = nullptr;
    SDL_Renderer*   m_pWindowRenderer = nullptr;
    TTF_Font*       m_pFont = nullptr;

    SDL_Event       m_windowEvent;

    string          m_sTitle;
    int             m_iWindow_X = 0;
    int             m_iWindow_Y = 0;
    int             m_iWindow_W = 800;
    int             m_iWindow_H = 600;
    int             m_iWindowID = 0;
    bool            m_bWindow_Ontop = false;
    bool            m_bWindow_Created = false;
    bool            m_bWindowShown = false;

    string          m_sBackgroundImage;
    bool            m_bShowPositionInfo = false;
    bool            m_bWindowsMovable = false;
    bool            m_bFlipImageHorizontally = false;
    bool            m_bFlipImageVertically = false;
    bool            m_bDefaultSize_W = false;
    bool            m_bDefaultSize_H = false;
};
