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

    m_pSharedMemory = new CSharedMemory(m_bNetworked, m_sIpAddress, m_iPort);

    if (!SetupSDL())
        return;
}


Application::~Application()
{
    delete m_pHUDWindow;
    delete m_pPFLWindow;
    delete m_pDEDWindow;
    delete m_pRWRWindow;
    delete m_pMFDLEFTWindow;
    delete m_pMFDRIGHTWindow;
    delete m_pHMSWindow;

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

    m_iFrameRate = 30;
    m_pConfigReader->getValue("FPS", m_iFrameRate);

    LoadBackground();

    bool keep_window_open = true;
    while (keep_window_open)
    {
        while (SDL_PollEvent(&m_windowEvent) > 0)
        {
            switch (m_windowEvent.type)
            {
                case SDL_SYSWMEVENT:
                    //if (windowEvent.syswm.msg->msg.win.msg == WM_USER + 1)
                    //{
                        //if (LOWORD(windowEvent.syswm.msg->msg.win.lParam) == WM_RBUTTONDBLCLK)
                        //{
                            // Show menu
                            //SDL_ShowWindow(window);
                            //SDL_RestoreWindow(window);
                        //}
                    //}                
                break;
                case SDL_QUIT:
                    keep_window_open = false;
                    break;
                case SDL_WINDOWEVENT:
                    //if (e.window.event == SDL_WINDOWEVENT_MINIMIZED)
                    //    SDL_HideWindow(window);
                    break;
            }
        }

        Update();
        Draw();
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
        SCREEN_SIZE_X, SCREEN_SIZE_Y,
        /* SDL_WINDOW_OPENGL |*/ SDL_WINDOW_SHOWN);

    if (!m_pWindow)
    {
        LOG_ERROR("Application::Application() Failed to create window");
        CLogger::getInstance()->error("Application::Application() SDL2 Error: %s", SDL_GetError());
        return false;
    }

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
        m_iFps = m_iFrameCount;
        m_iFrameCount = 0;
    }

    m_pSharedMemory->Update();
}

void Application::Draw()
{
    SDL_RenderClear(m_pWindowRenderer);

    m_iFrameCount++;
    m_iTimerFPS = SDL_GetTicks() - m_iLastFrame;
    if (m_iTimerFPS < (1000 / m_iFrameRate))
    {
        if (m_pImage == NULL)
            DrawBackground();
    
        UpdateDisplays();
    }

    SDL_RenderPresent(m_pWindowRenderer);
}

void Application::UpdateDisplays()
{
    if (m_bUseHUD)
    {
        if (!m_pHUDWindow)
            m_pHUDWindow = new CDisplayWindow("HUD", m_iHUD_X, m_iHUD_Y, m_iHUD_W, m_iHUD_H, m_iHUD_ONTOP);
        else
        {
            if (m_pHUDWindow->IsClosed())
                m_pHUDWindow->ShowWindow();

            m_pHUDWindow->MoveWindow(m_iHUD_X, m_iHUD_Y, m_iHUD_W, m_iHUD_H, m_iHUD_ONTOP);
            m_pHUDWindow->Update();
        }
    }

    if (m_bUsePFL)
    {
        if (!m_pPFLWindow)
            m_pPFLWindow = new CDisplayWindow("PFL", m_iPFL_X, m_iPFL_Y, m_iPFL_W, m_iPFL_H, m_iPFL_ONTOP);
        else
        {
            if (m_pPFLWindow->IsClosed())
                m_pPFLWindow->ShowWindow();

            m_pPFLWindow->MoveWindow(m_iPFL_X, m_iPFL_Y, m_iPFL_W, m_iPFL_H, m_iPFL_ONTOP);
            m_pPFLWindow->Update();
        }
    }

    if (m_bUseDED)
    {
        if (!m_pDEDWindow)
            m_pDEDWindow = new CDisplayWindow("DED", m_iDED_X, m_iDED_Y, m_iDED_W, m_iDED_H, m_iDED_ONTOP);
        else
        {
            if (m_pDEDWindow->IsClosed())
                m_pDEDWindow->ShowWindow();

            m_pDEDWindow->MoveWindow(m_iDED_X, m_iDED_Y, m_iDED_W, m_iDED_H, m_iDED_ONTOP);
            m_pDEDWindow->Update();
        }
    }

    if (m_bUseRWR)
    {
        if (!m_pRWRWindow)
            m_pRWRWindow = new CDisplayWindow("RWR", m_iRWR_X, m_iRWR_Y, m_iRWR_W, m_iRWR_H, m_iRWR_ONTOP);
        else
        {
            if (m_pRWRWindow->IsClosed())
                m_pRWRWindow->ShowWindow();

            m_pRWRWindow->MoveWindow(m_iRWR_X, m_iRWR_Y, m_iRWR_W, m_iRWR_H, m_iRWR_ONTOP);
            m_pRWRWindow->Update();
        }
    }

    if (m_bUseMFDLEFT)
    {
        if (!m_pMFDLEFTWindow)
            m_pMFDLEFTWindow = new CDisplayWindow("LEFT MFD", m_iMFDLEFT_X, m_iMFDLEFT_Y, m_iMFDLEFT_W, m_iMFDLEFT_H, m_iMFDLEFT_ONTOP);
        else
        {
            if (m_pMFDLEFTWindow->IsClosed())
                m_pMFDLEFTWindow->ShowWindow();

            m_pMFDLEFTWindow->MoveWindow(m_iMFDLEFT_X, m_iMFDLEFT_Y, m_iMFDLEFT_W, m_iMFDLEFT_H, m_iMFDLEFT_ONTOP);
            m_pMFDLEFTWindow->Update();
        }
    }

    if (m_bUseMFDRIGHT)
    {
        if (!m_pMFDRIGHTWindow)
            m_pMFDRIGHTWindow = new CDisplayWindow("RIGHT MFD", m_iMFDRIGHT_X, m_iMFDRIGHT_Y, m_iMFDRIGHT_W, m_iMFDRIGHT_H, m_iMFDRIGHT_ONTOP);
        else
        {
            if (m_pMFDRIGHTWindow->IsClosed())
                m_pMFDRIGHTWindow->ShowWindow();

            m_pMFDRIGHTWindow->MoveWindow(m_iMFDRIGHT_X, m_iMFDRIGHT_Y, m_iMFDRIGHT_W, m_iMFDRIGHT_H, m_iMFDRIGHT_ONTOP);
            m_pMFDRIGHTWindow->Update();
        }
    }

    if (m_bUseHMS)
    {
        if (!m_pHMSWindow)
            m_pHMSWindow = new CDisplayWindow("HMS", m_iHMS_X, m_iHMS_Y, m_iHMS_W, m_iHMS_H, m_iHMS_ONTOP);
        else
        {
            if (m_pHMSWindow->IsClosed())
                m_pHMSWindow->ShowWindow();

            m_pHMSWindow->MoveWindow(m_iHMS_X, m_iHMS_Y, m_iHMS_W, m_iHMS_H, m_iHMS_ONTOP);
            m_pHMSWindow->Update();
        }
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

    m_pConfigReader->getValue("NETWORKED", m_bNetworked);
    CLogger::getInstance()->debug("Application::Application() Networked : %s", m_bNetworked ? "true" : "false");

    m_pConfigReader->getValue("HOST", m_sIpAddress);
    CLogger::getInstance()->debug("Application::Application() HOST : %s", m_sIpAddress.c_str());

    m_pConfigReader->getValue("PORT", m_iPort);
    CLogger::getInstance()->debug("Application::Application() PORT : %d", m_iPort);

    m_pConfigReader->getValue("USE_HUD", m_bUseHUD);
    CLogger::getInstance()->debug("HUD : %s", m_bUseHUD ? "true" : "false");

    m_pConfigReader->getValue("USE_PFL", m_bUsePFL);
    CLogger::getInstance()->debug("PFL : %s", m_bUsePFL ? "true" : "false");

    m_pConfigReader->getValue("USE_DED", m_bUseDED);
    CLogger::getInstance()->debug("DED : %s", m_bUseDED ? "true" : "false");

    m_pConfigReader->getValue("USE_RWR", m_bUseRWR);
    CLogger::getInstance()->debug("RWR : %s", m_bUseRWR ? "true" : "false");

    m_pConfigReader->getValue("USE_MFDLEFT", m_bUseMFDLEFT);
    CLogger::getInstance()->debug("LEFT MFD : %s", m_bUseMFDLEFT ? "true" : "false");

    m_pConfigReader->getValue("USE_MFDRIGHT", m_bUseMFDRIGHT);
    CLogger::getInstance()->debug("RIGHT MFD : %s", m_bUseMFDRIGHT ? "true" : "false");

    m_pConfigReader->getValue("USE_HMS", m_bUseHMS);
    CLogger::getInstance()->debug("HMS : %s", m_bUseHMS ? "true" : "false");
}