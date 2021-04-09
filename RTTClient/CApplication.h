#pragma once

// std
#include <stdio.h>
#include <iostream>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "CSharedMemory.h"
#include "CConfigReader.h"

#define RTTCLient_VERSION_MAJOR @RTTClient_VERSION_MAJOR@
#define RTTClient_VERSION_MINOR @RTTClient_VERSION_MINOR@

using namespace std;
using namespace RTTClient::Common;

class Application
{
public:
    Application();
    ~Application();

    void Loop();

private:
    void update();
    void draw();
    void loadbackground();
    void drawbackground();

    SDL_Surface* m_pImage;
    SDL_Rect     m_imagePosition;

    SDL_Window*   m_pWindow;
//    SDL_Surface*  m_pWindowSurface;
    SDL_Renderer* m_pWindowRenderer;
    SDL_Event     m_windowEvent;

    CSharedMemory* m_pSharedMemory;
    CConfigReader* m_pConfigReader;

    int m_iFrameRate;
    int m_iLastTime;
    int m_iFrameCount;
    int m_iTimerFPS;
    int m_iLastFrame;
    int m_iFps;
};