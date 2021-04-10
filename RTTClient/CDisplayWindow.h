#pragma once

#include <string>
#include <iostream>
#include "CTools.h"

#include <SDL2/SDL.h>

using namespace std;

class CDisplayWindow
{
public:
    CDisplayWindow(const string& title, int x, int y, int width, int height, bool ontop);
    virtual ~CDisplayWindow();

    void SetBackgroundImage(string filename);

    void Render();
    void Clear() const;
    
    bool HandleEvents(SDL_Event& event);

    void ShowWindow();
    void MoveWindow(int x, int y, int width, int height, bool ontop);
    void CloseWindow();

    inline bool IsClosed() const { return !m_bWindowShown; }
    inline bool IsCreated() const { return m_bWindow_Created; }

private:
    bool Init();

    void DrawDefaultBackground();

    //SDL_Surface* horizontal_mirror(SDL_Surface* sfc);
    //SDL_Surface* flip_vertically(SDL_Surface* sfc);

    SDL_Window* m_pWindow = nullptr;
    SDL_Renderer*   m_pWindowRenderer = nullptr;
    //SDL_GLContext   m_pGLContext;

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
};
