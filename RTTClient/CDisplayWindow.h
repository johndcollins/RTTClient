#pragma once

#include <string>
#include <iostream>
#include "CTools.h"

#include <SDL2/SDL.h>

class CDisplayWindow
{
public:
    CDisplayWindow(const std::string& title, int x, int y, int width, int height, bool ontop);
    virtual ~CDisplayWindow();

    void Update();
    void Clear() const;
    
    void ShowWindow();
    void MoveWindow(int x, int y, int width, int height, bool ontop);

    inline bool IsClosed() const { return m_bWindow_Closed; }

    static SDL_Renderer* m_pRenderer;

private:
    bool Init();
    //SDL_Surface* horizontal_mirror(SDL_Surface* sfc);
    //SDL_Surface* flip_vertically(SDL_Surface* sfc);

    SDL_Event m_windowEvent;

    std::string m_sTitle;
    int m_iWindow_X = 0;
    int m_iWindow_Y = 0;
    int m_iWindow_W = 640;
    int m__iWindow_H = 480;
    bool m_bWindow_Ontop = false;
    bool m_bWindow_Closed = false;

    SDL_Window* m_pWindow = nullptr;
    SDL_GLContext m_pGLContext;
};
