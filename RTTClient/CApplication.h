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

    void ReadSettings();

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
    string          m_sIpAddress = "";
    int             m_iPort = 0;

    string          m_sHUDBackgroundImage = "";
    CDisplayWindow* m_pHUDWindow = nullptr;
    bool            m_bUseHUD = false;
    bool            m_bHUDFlippedVertically = false;
    bool            m_bHUDFlippedHorizontically = false;
    int             m_iHUD_X = 0;
    int             m_iHUD_Y = 0;
    int             m_iHUD_W = 0;
    int             m_iHUD_H = 0;
    bool            m_bHUD_ONTOP;
    bool            m_bHUDDefaultSize_W = false;
    bool            m_bHUDDefaultSize_H = false;

    string          m_sPFLBackgroundImage = "";
    CDisplayWindow* m_pPFLWindow = nullptr;
    bool            m_bUsePFL = false;
    bool            m_bPFLFlippedVertically = false;
    bool            m_bPFLFlippedHorizontically = false;
    int             m_iPFL_X = 0;
    int             m_iPFL_Y = 0;
    int             m_iPFL_W = 0;
    int             m_iPFL_H = 0;
    bool            m_bPFL_ONTOP = false;
    bool            m_bPFLDefaultSize_W = false;
    bool            m_bPFLDefaultSize_H = false;

    string          m_sDEDBackgroundImage = "";
    CDisplayWindow* m_pDEDWindow = nullptr;
    bool            m_bUseDED = false;
    bool            m_bDEDFlippedVertically = false;
    bool            m_bDEDFlippedHorizontically = false;
    int             m_iDED_X = 0;
    int             m_iDED_Y = 0;
    int             m_iDED_W = 0;
    int             m_iDED_H = 0;
    bool            m_bDED_ONTOP = false;
    bool            m_bDEDDefaultSize_W = false;
    bool            m_bDEDDefaultSize_H = false;

    string          m_sRWRBackgroundImage = "";
    CDisplayWindow* m_pRWRWindow = nullptr;
    bool            m_bUseRWR = false;
    bool            m_dRWRGrid = 0;
    bool            m_bRWRFlippedVertically = false;
    bool            m_bRWRFlippedHorizontically = false;
    int             m_iRWR_X = 0;
    int             m_iRWR_Y = 0;
    int             m_iRWR_W = 0;
    int             m_iRWR_H = 0;
    bool            m_bRWR_ONTOP = false;
    bool            m_bRWRDefaultSize_W = false;
    bool            m_bRWRDefaultSize_H = false;

    string          m_sMFDLEFTBackgroundImage = "";
    CDisplayWindow* m_pMFDLEFTWindow = nullptr;
    bool            m_bUseMFDLEFT = false;
    bool            m_bMFDLEFTFlippedVertically = false;
    bool            m_bMFDLEFTFlippedHorizontically = false;
    int             m_iMFDLEFT_X = 0;
    int             m_iMFDLEFT_Y = 0;
    int             m_iMFDLEFT_W = 0;
    int             m_iMFDLEFT_H = 0;
    bool            m_bMFDLEFT_ONTOP = false;
    bool            m_bMFDLEFTDefaultSize_W = false;
    bool            m_bMFDLEFTDefaultSize_H = false;

    string          m_sMFDRIGHTBackgroundImage = "";
    CDisplayWindow* m_pMFDRIGHTWindow = nullptr;
    bool            m_bUseMFDRIGHT = false;
    bool            m_bMFDRIGHTFlippedVertically = false;
    bool            m_bMFDRIGHTFlippedHorizontically = false;
    int             m_iMFDRIGHT_X = 0;
    int             m_iMFDRIGHT_Y = 0;
    int             m_iMFDRIGHT_W = 0;
    int             m_iMFDRIGHT_H = 0;
    bool            m_bMFDRIGHT_ONTOP = false;
    bool            m_bMFDRIGHTDefaultSize_W = false;
    bool            m_bMFDRIGHTDefaultSize_H = false;

    string          m_sHMSBackgroundImage = "";
    CDisplayWindow* m_pHMSWindow = nullptr;
    bool            m_bUseHMS = false;
    bool            m_bHMSFlippedVertically = false;
    bool            m_bHMSFlippedHorizontically = false;
    int             m_iHMS_X = 0;
    int             m_iHMS_Y = 0;
    int             m_iHMS_W = 0;
    int             m_iHMS_H = 0;
    bool            m_bHMS_ONTOP = false;
    bool            m_bHMSDefaultSize_W = false;
    bool            m_bHMSDefaultSize_H = false;

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