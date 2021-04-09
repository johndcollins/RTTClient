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
    void Draw();
    void UpdateDisplays();

    bool SetupSDL();

    void LoadBackground();
    void DrawBackground();

    void ReadSettings();

    SDL_Surface* m_pImage;
    SDL_Rect     m_imagePosition;

    SDL_Window*   m_pWindow;
//    SDL_Surface*  m_pWindowSurface;
    SDL_Renderer* m_pWindowRenderer;
    SDL_Event     m_windowEvent;

    CSharedMemory* m_pSharedMemory;
    CConfigReader* m_pConfigReader;

    bool m_bNetworked;
    string m_sIpAddress;
    int m_iPort;

    CDisplayWindow* m_pHUDWindow;
    bool m_bUseHUD;
    int m_iHUD_X;
    int m_iHUD_Y;
    int m_iHUD_W;
    int m_iHUD_H;
    int m_iHUD_ONTOP;

    CDisplayWindow* m_pPFLWindow;
    bool m_bUsePFL;
    int m_iPFL_X;
    int m_iPFL_Y;
    int m_iPFL_W;
    int m_iPFL_H;
    int m_iPFL_ONTOP;

    CDisplayWindow* m_pDEDWindow;
    bool m_bUseDED;
    int m_iDED_X;
    int m_iDED_Y;
    int m_iDED_W;
    int m_iDED_H;
    int m_iDED_ONTOP;

    CDisplayWindow* m_pRWRWindow;
    bool m_bUseRWR;
    bool m_dRWRGrid;
    int m_iRWR_X;
    int m_iRWR_Y;
    int m_iRWR_W;
    int m_iRWR_H;
    int m_iRWR_ONTOP;

    CDisplayWindow* m_pMFDLEFTWindow;
    bool m_bUseMFDLEFT;
    int m_iMFDLEFT_X;
    int m_iMFDLEFT_Y;
    int m_iMFDLEFT_W;
    int m_iMFDLEFT_H;
    int m_iMFDLEFT_ONTOP;

    CDisplayWindow* m_pMFDRIGHTWindow;
    bool m_bUseMFDRIGHT;
    int m_iMFDRIGHT_X;
    int m_iMFDRIGHT_Y;
    int m_iMFDRIGHT_W;
    int m_iMFDRIGHT_H;
    int m_iMFDRIGHT_ONTOP;

    CDisplayWindow* m_pHMSWindow;
    bool m_bUseHMS;
    int m_iHMS_X;
    int m_iHMS_Y;
    int m_iHMS_W;
    int m_iHMS_H;
    int m_iHMS_ONTOP;

    bool m_bDataF4;
    bool m_bDataBMS;
    bool m_bDataOSB;
    bool m_bDataIVIBE;

    int m_iFrameRate;
    int m_iLastTime;
    int m_iFrameCount;
    int m_iTimerFPS;
    int m_iLastFrame;
    int m_iFps;
};