#include "CApplication.h"
#include "CConfigReader.h"
#include "CLogger.h"

#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#include <vector>

using namespace std;
using namespace RTTClient::Common;

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 600

Application::Application(int argc, char* argv[])
{
    CLogger::getInstance()->updateLogLevel(LogLevel::LOG_LEVEL_DEBUG);

    ReadSettings();

    m_pSharedMemory = new CSharedMemory(m_iFps, m_bNetworked, m_sIpAddress, m_iPort);

    if (!SetupSDL())
        return;
}


Application::~Application()
{
    if (m_pHUDWindow != nullptr)
        delete m_pHUDWindow;
    if (m_pPFLWindow != nullptr)
        delete m_pPFLWindow;
    if (m_pDEDWindow != nullptr)
        delete m_pDEDWindow;
    if (m_pRWRWindow != nullptr)
        delete m_pRWRWindow;
    if (m_pMFDLEFTWindow != nullptr)
        delete m_pMFDLEFTWindow;
    if (m_pMFDRIGHTWindow != nullptr)
        delete m_pMFDRIGHTWindow;
    if (m_pHMSWindow != nullptr)
        delete m_pHMSWindow;

    if (m_pSharedMemory != nullptr)
        delete m_pSharedMemory;

    //SDL_FreeSurface(m_pWindowSurface);
    SDL_DestroyRenderer(m_pWindowRenderer);
    SDL_DestroyWindow(m_pWindow);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Application::Loop()
{
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    LoadBackground();

    bool keep_window_open = true;
    while (keep_window_open)
    {
        while (SDL_PollEvent(&m_windowEvent) > 0)
        {
            //User requests quit
            if (m_windowEvent.type == SDL_WINDOWEVENT && (m_windowEvent.type == SDL_QUIT || m_windowEvent.window.event == SDL_WINDOWEVENT_CLOSE))
            {
                LOG_DEBUG("Application::Loop() SDL_QUIT");
                keep_window_open = false;
            } 
            else if (m_windowEvent.type == SDL_WINDOWEVENT && m_windowEvent.window.windowID == m_iWindowID)
            {
                switch (m_windowEvent.window.event)
                {
                    //Window appeared
                case SDL_WINDOWEVENT_SHOWN:
                    m_bShown = true;
                    break;
                    //Window disappeared
                case SDL_WINDOWEVENT_HIDDEN:
                    m_bShown = false;
                    break;
                    //Get new dimensions and repaint
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    m_iWindowWidth = m_windowEvent.window.data1;
                    m_iWindowHeight = m_windowEvent.window.data2;
                    SDL_RenderPresent(m_pWindowRenderer);
                    break;
                //Repaint on expose
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_RenderPresent(m_pWindowRenderer);
                break;
                case SDL_WINDOWEVENT_CLOSE:
                    LOG_DEBUG("Application::Loop() SDL_WINDOWEVENT_CLOSE");
                    SDL_HideWindow(m_pWindow);
                    keep_window_open = false;
                    break;
                }
            }
            else
            { 
                UpdateDisplaysEvent(m_windowEvent);
            }
        }

        Update();
        Render();
    }
}

bool Application::SetupSDL()
{
    SDL_Init(0);

    LOG_DEBUG("Application::Application() Testing Video Drivers...");
    std::vector< bool > drivers(SDL_GetNumVideoDrivers());
    for (int i = 0; i < drivers.size(); ++i)
    {
        drivers[i] = (0 == SDL_VideoInit(SDL_GetVideoDriver(i)));
        SDL_VideoQuit();
    }

    LOG_DEBUG("Application::Application() SDL_VIDEODRIVER available:");
    for (int i = 0; i < drivers.size(); ++i)
    {
        CLogger::getInstance()->debug("  %s", SDL_GetVideoDriver(i));
    }
    LOG_DEBUG("");

    LOG_DEBUG("Application::Application() SDL_VIDEODRIVER usable:");
    for (int i = 0; i < drivers.size(); ++i)
    {
        if (!drivers[i]) continue;
        CLogger::getInstance()->debug("  %s", SDL_GetVideoDriver(i));
    }
    LOG_DEBUG("");

    // ----- Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        LOG_ERROR("Application::Application() SDL could not initialize.");
        CLogger::getInstance()->error("Application::Application() SDL could not initialize. %s", SDL_GetError());
        return false;
    }
    else
        LOG_DEBUG("Application::Application() SDL Initialized.");

    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG))
    {
        CLogger::getInstance()->error("Application::Application() Error, while intitializing IMG PNG and IMG JPG. %s", IMG_GetError());
        return false;
    }
    else
        LOG_DEBUG("Application::Application() IMG Initialized.");

    if (TTF_Init() == -1)
    {
        CLogger::getInstance()->error("Application::Application() Failed to initialize SDL_TTF. %s", TTF_GetError());
        return false;
    }
    else
        LOG_DEBUG("Application::Application() TTF Initialized.");

    CLogger::getInstance()->debug("Application::Application() SDL_VIDEODRIVER selected : %s", SDL_GetCurrentVideoDriver());

    m_pWindow = SDL_CreateWindow("RTTClient",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_SIZE_X, SCREEN_SIZE_Y, SDL_WINDOW_MINIMIZED);

    if (!m_pWindow)
    {
        LOG_ERROR("Application::Application() Failed to create window");
        CLogger::getInstance()->error("Application::Application() SDL2 Error: %s", SDL_GetError());
        return false;
    }
    
    m_bShown = true;
    m_iWindowID = SDL_GetWindowID(m_pWindow);

    LOG_DEBUG("Application::Application() SDL_RENDER_DRIVER available:");
    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);
        CLogger::getInstance()->debug("  %s", info.name);
    }
    LOG_DEBUG("");

    //m_pWindowSurface = SDL_GetWindowSurface(m_pWindow);

    //if (!m_pWindowSurface)
    //{
    //    LOG_ERROR("Application::Application() Failed to get window's surface");
    //    CLogger::getInstance()->error("Application::Application() SDL2 Error: %x", SDL_GetError());
    //    return;
    //}

    m_pWindowRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_pWindowRenderer)
    {
        LOG_ERROR("Application::Application() Failed to get window's surface");
        CLogger::getInstance()->error("Application::Application() SDL2 Error: %s", SDL_GetError());
        return false;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(m_pWindowRenderer, &info);
    CLogger::getInstance()->debug("Application::Application() SDL_RENDER_DRIVER selected : %s", info.name);

    return true;
}

void Application::LoadBackground()
{
    string filename("");
    m_pConfigReader->getValue("BACKGOUND_IMG", filename);

    m_pImage = NULL;
    if (filename != "")
        m_pImage = IMG_Load(filename.c_str());

    m_imagePosition.x = 0;
    m_imagePosition.y = 0;
    m_imagePosition.w = SCREEN_SIZE_X;
    m_imagePosition.h = SCREEN_SIZE_Y;
}

void Application::DrawBackground()
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = SCREEN_SIZE_X;
    rect.h = SCREEN_SIZE_Y;

    SDL_SetRenderDrawColor(m_pWindowRenderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(m_pWindowRenderer, &rect);

    SDL_SetRenderDrawColor(m_pWindowRenderer, 0, 0, 0, 255);
}

void Application::Update()
{
    m_iLastFrame = SDL_GetTicks();
    if (m_iLastFrame >= (m_iLastFrame + 1000))
    {
        m_iLastTime = m_iLastFrame;
        m_iFpsActual = m_iFrameCount;
        m_iFrameCount = 0;
    }


    int serverFps = m_pSharedMemory->GetFps();
    if (serverFps != 0)
        m_iFps = serverFps;

    m_pSharedMemory->Update();
}

void Application::Render()
{
    SDL_RenderClear(m_pWindowRenderer);

    m_iFrameCount++;
    m_iTimerFPS = SDL_GetTicks() - m_iLastFrame;
    if (m_iTimerFPS < (1000 / m_iFps))
    {
        if (m_pImage == NULL)
            DrawBackground();
    
        RenderDisplays();
    }

    SDL_RenderPresent(m_pWindowRenderer);
}

void Application::UpdateDisplaysEvent(SDL_Event &event)
{
    if (m_bUseHUD)
    {
        if (m_pHUDWindow != nullptr)
            m_pHUDWindow->HandleEvents(event);
    }

    if (m_bUsePFL)
    {
        if (m_pPFLWindow != nullptr)
            m_pPFLWindow->HandleEvents(event);
    }

    if (m_bUseDED)
    {
        if (m_pDEDWindow != nullptr)
            m_pDEDWindow->HandleEvents(event);
    }

    if (m_bUseRWR)
    {
        if (m_pRWRWindow != nullptr)
            m_pRWRWindow->HandleEvents(event);
    }

    if (m_bUseMFDLEFT)
    {
        if (m_pMFDLEFTWindow != nullptr)
            m_pMFDLEFTWindow->HandleEvents(event);
    }

    if (m_bUseMFDRIGHT)
    {
        if (m_pMFDRIGHTWindow != nullptr)
            m_pMFDRIGHTWindow->HandleEvents(event);
    }

    if (m_bUseHMS)
    {
        if (m_pHMSWindow != nullptr)
            m_pHMSWindow->HandleEvents(event);
    }
}

void Application::RenderDisplays()
{
    if (m_bUseHUD)
    {
        if (m_pHUDWindow == nullptr)
            m_pHUDWindow = new CDisplayWindow("HUD", m_iHUD_X, m_iHUD_Y, m_iHUD_W, m_iHUD_H, m_iHUD_ONTOP);
        else
        {
            if (m_pHUDWindow->IsClosed())
                m_pHUDWindow->ShowWindow();

            m_pHUDWindow->MoveWindow(m_iHUD_X, m_iHUD_Y, m_iHUD_W, m_iHUD_H, m_iHUD_ONTOP);
            m_pHUDWindow->Render();
        }
    }

    if (m_bUsePFL)
    {
        if (m_pPFLWindow == nullptr)
            m_pPFLWindow = new CDisplayWindow("PFL", m_iPFL_X, m_iPFL_Y, m_iPFL_W, m_iPFL_H, m_iPFL_ONTOP);
        else
        {
            if (m_pPFLWindow->IsClosed())
                m_pPFLWindow->ShowWindow();

            m_pPFLWindow->MoveWindow(m_iPFL_X, m_iPFL_Y, m_iPFL_W, m_iPFL_H, m_iPFL_ONTOP);
            m_pPFLWindow->Render();
        }
    }

    if (m_bUseDED)
    {
        if (m_pDEDWindow == nullptr)
            m_pDEDWindow = new CDisplayWindow("DED", m_iDED_X, m_iDED_Y, m_iDED_W, m_iDED_H, m_iDED_ONTOP);
        else
        {
            if (m_pDEDWindow->IsClosed())
                m_pDEDWindow->ShowWindow();

            m_pDEDWindow->MoveWindow(m_iDED_X, m_iDED_Y, m_iDED_W, m_iDED_H, m_iDED_ONTOP);
            m_pDEDWindow->Render();
        }
    }

    if (m_bUseRWR)
    {
        if (m_pRWRWindow == nullptr)
            m_pRWRWindow = new CDisplayWindow("RWR", m_iRWR_X, m_iRWR_Y, m_iRWR_W, m_iRWR_H, m_iRWR_ONTOP);
        else
        {
            if (m_pRWRWindow->IsClosed())
                m_pRWRWindow->ShowWindow();

            m_pRWRWindow->MoveWindow(m_iRWR_X, m_iRWR_Y, m_iRWR_W, m_iRWR_H, m_iRWR_ONTOP);
            m_pRWRWindow->Render();
        }
    }

    if (m_bUseMFDLEFT)
    {
        if (m_pMFDLEFTWindow == nullptr)
            m_pMFDLEFTWindow = new CDisplayWindow("LEFT MFD", m_iMFDLEFT_X, m_iMFDLEFT_Y, m_iMFDLEFT_W, m_iMFDLEFT_H, m_iMFDLEFT_ONTOP);
        else
        {
            if (m_pMFDLEFTWindow->IsClosed())
                m_pMFDLEFTWindow->ShowWindow();

            m_pMFDLEFTWindow->MoveWindow(m_iMFDLEFT_X, m_iMFDLEFT_Y, m_iMFDLEFT_W, m_iMFDLEFT_H, m_iMFDLEFT_ONTOP);
            m_pMFDLEFTWindow->Render();
        }
    }

    if (m_bUseMFDRIGHT)
    {
        if (m_pMFDRIGHTWindow == nullptr)
            m_pMFDRIGHTWindow = new CDisplayWindow("RIGHT MFD", m_iMFDRIGHT_X, m_iMFDRIGHT_Y, m_iMFDRIGHT_W, m_iMFDRIGHT_H, m_iMFDRIGHT_ONTOP);
        else
        {
            if (m_pMFDRIGHTWindow->IsClosed())
                m_pMFDRIGHTWindow->ShowWindow();

            m_pMFDRIGHTWindow->MoveWindow(m_iMFDRIGHT_X, m_iMFDRIGHT_Y, m_iMFDRIGHT_W, m_iMFDRIGHT_H, m_iMFDRIGHT_ONTOP);
            m_pMFDRIGHTWindow->Render();
        }
    }

    if (m_bUseHMS)
    {
        if (m_pHMSWindow == nullptr)
            m_pHMSWindow = new CDisplayWindow("HMS", m_iHMS_X, m_iHMS_Y, m_iHMS_W, m_iHMS_H, m_iHMS_ONTOP);
        else
        {
            if (m_pHMSWindow->IsClosed())
                m_pHMSWindow->ShowWindow();

            m_pHMSWindow->MoveWindow(m_iHMS_X, m_iHMS_Y, m_iHMS_W, m_iHMS_H, m_iHMS_ONTOP);
            m_pHMSWindow->Render();
        }
    }
}

void Application::CloseDisplays()
{
    if (m_bUseHUD)
    {
        if (m_pHUDWindow != nullptr)
            m_pHUDWindow->CloseWindow();
    }

    if (m_bUsePFL)
    {
        if (m_pPFLWindow != nullptr)
            m_pPFLWindow->CloseWindow();
    }

    if (m_bUseDED)
    {
        if (m_pDEDWindow != nullptr)
            m_pDEDWindow->CloseWindow();
    }

    if (m_bUseRWR)
    {
        if (m_pRWRWindow != nullptr)
            m_pRWRWindow->CloseWindow();
    }

    if (m_bUseMFDLEFT)
    {
        if (m_pMFDLEFTWindow != nullptr)
            m_pMFDLEFTWindow->CloseWindow();
    }

    if (m_bUseMFDRIGHT)
    {
        if (m_pMFDRIGHTWindow != nullptr)
            m_pMFDRIGHTWindow->CloseWindow();
    }

    if (m_bUseHMS)
    {
        if (m_pHMSWindow != nullptr)
            m_pHMSWindow->CloseWindow();
    }
}

void Application::ReadSettings()
{
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string path;
    if (count != -1)
        path = dirname(result);
    else
    {
        LOG_ERROR("Application::Application() Unable to locate exe directory");
        return;
    }

    path.append("/RTTClient.ini");

    ifstream f(path.c_str());
    if (!f.good())
    {
        CLogger::getInstance()->error("Application::Application() Unable to locate RTTClient.ini file. Path : %s", path);
        return;
    }

    m_pConfigReader = CConfigReader::getInstance();

    // parse the configuration file
    m_pConfigReader->parseFile(path);

    LOG_DEBUG("Application::Application() Configuration:");

    m_pConfigReader->getValue("NETWORKED", m_bNetworked);
    CLogger::getInstance()->debug("   Networked : %s", m_bNetworked ? "true" : "false");
    if (!m_bNetworked)
        LOG_ALWAYS("Warning! Networked is not set. This version is for Linux only and doesn't support reading shared memory directly.");

    m_pConfigReader->getValue("HOST", m_sIpAddress);
    CLogger::getInstance()->debug("   HOST : %s", m_sIpAddress.c_str());

    m_pConfigReader->getValue("PORT", m_iPort);
    CLogger::getInstance()->debug("   PORT : %d", m_iPort);

    m_pConfigReader->getValue("FPS", m_iFps);
    if (m_iFps <= 0)
        m_iFps = 30;
    CLogger::getInstance()->debug("   FPS : %d", m_iFps);

    m_pConfigReader->getValue("USE_HUD", m_bUseHUD);
    CLogger::getInstance()->debug("   HUD : %s", m_bUseHUD ? "true" : "false");

    m_pConfigReader->getValue("HUD_X", m_iHUD_X);
    CLogger::getInstance()->debug("   HUD X : %d", m_iHUD_X);

    m_pConfigReader->getValue("HUD_Y", m_iHUD_Y);
    CLogger::getInstance()->debug("   HUD Y : %d", m_iHUD_Y);

    m_pConfigReader->getValue("HUD_W", m_iHUD_W);
    if (m_iHUD_W <= 0)
        m_iHUD_W = 600;
    CLogger::getInstance()->debug("   HUD W : %d", m_iHUD_W);

    m_pConfigReader->getValue("HUD_H", m_iHUD_H);
    if (m_iHUD_H <= 0)
        m_iHUD_H = 600;
    CLogger::getInstance()->debug("   HUD H : %d", m_iHUD_H);

    m_pConfigReader->getValue("HUD_ONTOP", m_iHUD_ONTOP);
    CLogger::getInstance()->debug("   HUD ONTOP : %s", m_iHUD_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("USE_PFL", m_bUsePFL);
    CLogger::getInstance()->debug("   PFL : %s", m_bUsePFL ? "true" : "false");

    m_pConfigReader->getValue("PFL_X", m_iPFL_X);
    CLogger::getInstance()->debug("   PFL X : %d", m_iPFL_X);

    m_pConfigReader->getValue("PFL_Y", m_iPFL_Y);
    CLogger::getInstance()->debug("   PFL Y : %d", m_iPFL_Y);

    m_pConfigReader->getValue("PFL_W", m_iPFL_W);
    if (m_iPFL_W <= 0)
        m_iPFL_W = 600;
    CLogger::getInstance()->debug("   PFL W : %d", m_iPFL_W);

    m_pConfigReader->getValue("PFL_H", m_iPFL_H);
    if (m_iPFL_H <= 0)
        m_iPFL_H = 600;
    CLogger::getInstance()->debug("   PFL H : %d", m_iPFL_H);

    m_pConfigReader->getValue("PFL_ONTOP", m_iPFL_ONTOP);
    CLogger::getInstance()->debug("   PFL ONTOP : %s", m_iPFL_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("USE_DED", m_bUseDED);
    CLogger::getInstance()->debug("   DED : %s", m_bUseDED ? "true" : "false");

    m_pConfigReader->getValue("DED_X", m_iDED_X);
    CLogger::getInstance()->debug("   DED X : %d", m_iDED_X);

    m_pConfigReader->getValue("DED_Y", m_iDED_Y);
    CLogger::getInstance()->debug("   DED Y : %d", m_iDED_Y);

    m_pConfigReader->getValue("DED_W", m_iDED_W);
    if (m_iDED_W <= 0)
        m_iDED_W = 600;
    CLogger::getInstance()->debug("   DED W : %d", m_iDED_W);

    m_pConfigReader->getValue("DED_H", m_iDED_H);
    if (m_iDED_H <= 0)
        m_iDED_H = 600;
    CLogger::getInstance()->debug("   DED H : %d", m_iDED_H);

    m_pConfigReader->getValue("DED_ONTOP", m_iDED_ONTOP);
    CLogger::getInstance()->debug("   DED ONTOP : %s", m_iDED_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("USE_RWR", m_bUseRWR);
    CLogger::getInstance()->debug("   RWR : %s", m_bUseRWR ? "true" : "false");

    m_pConfigReader->getValue("RWR_X", m_iRWR_X);
    CLogger::getInstance()->debug("   RWR X : %d", m_iRWR_X);

    m_pConfigReader->getValue("RWR_Y", m_iRWR_Y);
    CLogger::getInstance()->debug("   RWR Y : %d", m_iRWR_Y);

    m_pConfigReader->getValue("RWR_W", m_iRWR_W);
    if (m_iRWR_W <= 0)
        m_iRWR_W = 600;
    CLogger::getInstance()->debug("   RWR W : %d", m_iRWR_W);

    m_pConfigReader->getValue("RWR_H", m_iRWR_H);
    if (m_iRWR_H <= 0)
        m_iRWR_H = 600;
    CLogger::getInstance()->debug("   RWR H : %d", m_iRWR_H);

    m_pConfigReader->getValue("RWR_ONTOP", m_iRWR_ONTOP);
    CLogger::getInstance()->debug("   RWR ONTOP : %s", m_iRWR_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("USE_MFDLEFT", m_bUseMFDLEFT);
    CLogger::getInstance()->debug("   LEFT MFD : %s", m_bUseMFDLEFT ? "true" : "false");
    
    m_pConfigReader->getValue("MFDLEFT_X", m_iMFDLEFT_X);
    CLogger::getInstance()->debug("   LEFT MFD X : %d", m_iMFDLEFT_X);

    m_pConfigReader->getValue("MFDLEFT_Y", m_iMFDLEFT_Y);
    CLogger::getInstance()->debug("   LEFT MFD Y : %d", m_iMFDLEFT_Y);

    m_pConfigReader->getValue("MFDLEFT_W", m_iMFDLEFT_W);
    if (m_iMFDLEFT_W <= 0)
        m_iMFDLEFT_W = 600;
    CLogger::getInstance()->debug("   LEFT MFD W : %d", m_iMFDLEFT_W);

    m_pConfigReader->getValue("MFDLEFT_H", m_iMFDLEFT_H);
    if (m_iMFDLEFT_H <= 0)
        m_iMFDLEFT_H = 600;
    CLogger::getInstance()->debug("   LEFT MFD H : %d", m_iMFDLEFT_H);

    m_pConfigReader->getValue("MFDLEFT_ONTOP", m_iMFDLEFT_ONTOP);
    CLogger::getInstance()->debug("   LEFT MFD ONTOP : %s", m_iMFDLEFT_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("USE_MFDRIGHT", m_bUseMFDRIGHT);
    CLogger::getInstance()->debug("   RIGHT MFD : %s", m_bUseMFDRIGHT ? "true" : "false");

    m_pConfigReader->getValue("MFDRIGHT_X", m_iMFDRIGHT_X);
    CLogger::getInstance()->debug("   RIGHT MFD X : %d", m_iMFDRIGHT_X);

    m_pConfigReader->getValue("MFDLEFT_Y", m_iMFDRIGHT_Y);
    CLogger::getInstance()->debug("   RIGHT MFD Y : %d", m_iMFDRIGHT_Y);

    m_pConfigReader->getValue("MFDLEFT_W", m_iMFDRIGHT_W);
    if (m_iMFDRIGHT_W <= 0)
        m_iMFDRIGHT_W = 800;
    CLogger::getInstance()->debug("   RIGHT MFD W : %d", m_iMFDRIGHT_W);

    m_pConfigReader->getValue("MFDLEFT_H", m_iMFDRIGHT_H);
    if (m_iMFDRIGHT_H <= 0)
        m_iMFDRIGHT_H = 600;
    CLogger::getInstance()->debug("   RIGHT MFD H : %d", m_iMFDRIGHT_H);

    m_pConfigReader->getValue("MFDLEFT_ONTOP", m_iMFDRIGHT_ONTOP);
    CLogger::getInstance()->debug("   RIGHT MFD ONTOP : %s", m_iMFDRIGHT_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("USE_HMS", m_bUseHMS);
    CLogger::getInstance()->debug("   HMS : %s", m_bUseHMS ? "true" : "false");
    m_pConfigReader->getValue("HMS_X", m_iHMS_X);
    CLogger::getInstance()->debug("   HMS X : %d", m_iHMS_X);

    m_pConfigReader->getValue("HMS_Y", m_iHMS_Y);
    CLogger::getInstance()->debug("   HMS Y : %d", m_iHMS_Y);

    m_pConfigReader->getValue("HMS_W", m_iHMS_W);
    if (m_iHMS_W <= 0)
        m_iHMS_W = 600;
    CLogger::getInstance()->debug("   HMS W : %d", m_iHMS_W);

    m_pConfigReader->getValue("HMS_H", m_iHMS_H);
    if (m_iHMS_H <= 0)
        m_iHMS_H = 600;
    CLogger::getInstance()->debug("   HMS H : %d", m_iHMS_H);

    m_pConfigReader->getValue("HMS_ONTOP", m_iHMS_ONTOP);
    CLogger::getInstance()->debug("   HMS ONTOP : %s", m_iHMS_ONTOP ? "true" : "false");
}