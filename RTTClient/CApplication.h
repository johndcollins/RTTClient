#pragma once

// std
#include <stdio.h>
#include <iostream>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// opengl
#include <GL/glew.h>

#include "CSharedMemory.h"
#include "CConfigReader.h"
#include "CDisplayWindow.h"

#define RTTCLient_VERSION_MAJOR @RTTClient_VERSION_MAJOR@
#define RTTClient_VERSION_MINOR @RTTClient_VERSION_MINOR@

using namespace std;
using namespace RTTClient::Common;

class Application
{
public:
    Application(int argc, char* argv[]);
    ~Application();

    void Loop();

private:
    void Update();
    void Render();

    void UpdateDisplaysEvent(SDL_Event &event);
    void RenderDisplays();
    void CloseDisplays();

    bool SetupSDL();

    void LoadBackground();
    void DrawBackground();

    void ReadSettings();

    SDL_Surface* m_pImage = nullptr;
    SDL_Rect     m_imagePosition;

    SDL_Window* m_pWindow = nullptr;
    int m_iWindowID = 0;

//    SDL_Surface*  m_pWindowSurface;
    SDL_Renderer*   m_pWindowRenderer = nullptr;
    SDL_Event       m_windowEvent;

    CSharedMemory*  m_pSharedMemory = nullptr;
    CConfigReader*  m_pConfigReader = nullptr;

    bool            m_bShown = false;
    int             m_iWindowWidth = 800;
    int             m_iWindowHeight = 600;

    bool            m_bNetworked = false;
    string          m_sIpAddress;
    int             m_iPort = 0;

    CDisplayWindow* m_pHUDWindow = nullptr;
    bool            m_bUseHUD = false;
    int             m_iHUD_X = 0;
    int             m_iHUD_Y = 0;
    int             m_iHUD_W = 0;
    int             m_iHUD_H = 0;
    bool            m_iHUD_ONTOP;

    CDisplayWindow* m_pPFLWindow = nullptr;
    bool            m_bUsePFL = false;
    int             m_iPFL_X = 0;
    int             m_iPFL_Y = 0;
    int             m_iPFL_W = 0;
    int             m_iPFL_H = 0;
    bool            m_iPFL_ONTOP = false;

    CDisplayWindow* m_pDEDWindow = nullptr;
    bool            m_bUseDED = false;
    int             m_iDED_X = 0;
    int             m_iDED_Y = 0;
    int             m_iDED_W = 0;
    int             m_iDED_H = 0;
    bool            m_iDED_ONTOP = false;

    CDisplayWindow* m_pRWRWindow = nullptr;
    bool            m_bUseRWR = false;
    bool            m_dRWRGrid = 0;
    int             m_iRWR_X = 0;
    int             m_iRWR_Y = 0;
    int             m_iRWR_W = 0;
    int             m_iRWR_H = 0;
    bool            m_iRWR_ONTOP = false;

    CDisplayWindow* m_pMFDLEFTWindow = nullptr;
    bool            m_bUseMFDLEFT = false;
    int             m_iMFDLEFT_X = 0;
    int             m_iMFDLEFT_Y = 0;
    int             m_iMFDLEFT_W = 0;
    int             m_iMFDLEFT_H = 0;
    bool            m_iMFDLEFT_ONTOP = false;

    CDisplayWindow* m_pMFDRIGHTWindow = nullptr;
    bool            m_bUseMFDRIGHT = false;
    int             m_iMFDRIGHT_X = 0;
    int             m_iMFDRIGHT_Y = 0;
    int             m_iMFDRIGHT_W = 0;
    int             m_iMFDRIGHT_H = 0;
    bool            m_iMFDRIGHT_ONTOP = false;

    CDisplayWindow* m_pHMSWindow = nullptr;
    bool            m_bUseHMS = false;
    int             m_iHMS_X = 0;
    int             m_iHMS_Y = 0;
    int             m_iHMS_W = 0;
    int             m_iHMS_H = 0;
    bool            m_iHMS_ONTOP = false;

    bool            m_bDataF4 = false;
    bool            m_bDataBMS = false;
    bool            m_bDataOSB = false;
    bool            m_bDataIVIBE = false;

    int             m_iFrameRate = 0;
    int             m_iLastTime = 0;
    int             m_iFrameCount = 0;
    int             m_iTimerFPS = 0;
    int             m_iLastFrame = 0;
    int             m_iFpsActual = 0;
    int             m_iFps = 0;
};