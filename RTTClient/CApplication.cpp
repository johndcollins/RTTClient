#include "CApplication.h"
#include "CConfigReader.h"
#include "CLogger.h"

#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#include <vector>
#include <chrono>

using namespace std;
using namespace RTTClient::Common;

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 600

Application::Application(bool logDebug)
{
    LOG_DEBUG("Application::Application() Begin");

    if (logDebug)
        CLogger::getInstance()->updateLogLevel(LogLevel::LOG_LEVEL_DEBUG);
    else
        CLogger::getInstance()->updateLogLevel(LogLevel::DISABLE_LOG);

    ReadSettings();

    m_pSharedMemory = new CSharedMemory(m_iFps, m_bNetworked, m_sIpAddress, m_iPort);

    if (!SetupSDL())
    {
        LOG_ERROR("Application::Application() Failed to setup SDL");
        return;
    }

    LOG_DEBUG("Application::Application() End");
}


Application::~Application()
{
    LOG_DEBUG("Application::~Application() Begin");

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

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    LOG_DEBUG("Application::~Application() End");
}

void Application::Loop()
{
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    bool quit_game = false;

    while (!quit_game)
    {
        unsigned int frameStart = SDL_GetTicks();

        int fps = 30;

        int serverFps = m_pSharedMemory->GetFps();
        if (serverFps != 0)
            fps = serverFps;

        int frameDelay = fps / 1000;

        quit_game = ProcessInput();

        Update();
        Render();

        int frameTime = SDL_GetTicks() - frameStart;

        if (frameDelay > frameTime)
            SDL_Delay(frameDelay - frameTime);
    }
}

bool Application::SetupSDL()
{
    LOG_DEBUG("Application::SetupSDL() Begin");

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

    LOG_DEBUG("Application::Application() SDL_RENDER_DRIVER available:");
    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);
        CLogger::getInstance()->debug("  %s", info.name);
    }
    LOG_DEBUG("");

    LOG_DEBUG("Application::SetupSDL() End");

    return true;
}

bool Application::ProcessInput()
{
    while (SDL_PollEvent(&m_windowEvent) > 0)
    {
        //User requests quit
        if (m_windowEvent.type == SDL_WINDOWEVENT && (m_windowEvent.type == SDL_QUIT || m_windowEvent.window.event == SDL_WINDOWEVENT_CLOSE))
        {
            LOG_DEBUG("Application::Loop() SDL_QUIT");
            CloseDisplays();
            return true;
        }
        else
        {
            UpdateDisplaysEvent(m_windowEvent);
        }
    }

    return false;
}

void Application::Update()
{
    m_pSharedMemory->Update();
}

void Application::Render()
{
    RenderDisplays();
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
        {
            m_pHUDWindow = new CDisplayWindow("HUD", m_iHUD_X, m_iHUD_Y, m_iHUD_W, m_iHUD_H, m_bHUD_ONTOP);
            m_pHUDWindow->SetBackgroundImage(m_sHUDBackgroundImage);

            m_pHUDWindow->SetFlipImageVertically(m_bHUDFlippedVertically);
            m_pHUDWindow->SetFlipImageHorizontally(m_bHUDFlippedHorizontically);
            m_pHUDWindow->SetUseDefaultWidth(m_bHUDDefaultSize_W);
            m_pHUDWindow->SetUseDefaultHeight(m_bHUDDefaultSize_H);

            m_pHUDWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pHUDWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pHUDWindow->IsClosed())
                m_pHUDWindow->ShowWindow();

            m_pHUDWindow->MoveWindow(m_iHUD_X, m_iHUD_Y, m_iHUD_W, m_iHUD_H, m_bHUD_ONTOP);
            m_pHUDWindow->Render(m_pSharedMemory->Connected(),  m_pSharedMemory->DisplayImage(HUD));
        }
    }

    if (m_bUsePFL)
    {
        if (m_pPFLWindow == nullptr)
        {
            m_pPFLWindow = new CDisplayWindow("PFL", m_iPFL_X, m_iPFL_Y, m_iPFL_W, m_iPFL_H, m_bPFL_ONTOP);
            m_pPFLWindow->SetBackgroundImage(m_sPFLBackgroundImage);

            m_pPFLWindow->SetFlipImageVertically(m_bPFLFlippedVertically);
            m_pPFLWindow->SetFlipImageHorizontally(m_bPFLFlippedHorizontically);
            m_pPFLWindow->SetUseDefaultWidth(m_bPFLDefaultSize_W);
            m_pPFLWindow->SetUseDefaultHeight(m_bPFLDefaultSize_H);

            m_pPFLWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pPFLWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pPFLWindow->IsClosed())
                m_pPFLWindow->ShowWindow();

            m_pPFLWindow->MoveWindow(m_iPFL_X, m_iPFL_Y, m_iPFL_W, m_iPFL_H, m_bPFL_ONTOP);
            m_pPFLWindow->Render(m_pSharedMemory->Connected(), m_pSharedMemory->DisplayImage(PFL));
        }
    }

    if (m_bUseDED)
    {
        if (m_pDEDWindow == nullptr)
        {
            m_pDEDWindow = new CDisplayWindow("DED", m_iDED_X, m_iDED_Y, m_iDED_W, m_iDED_H, m_bDED_ONTOP);
            m_pDEDWindow->SetBackgroundImage(m_sDEDBackgroundImage);

            m_pDEDWindow->SetFlipImageVertically(m_bDEDFlippedVertically);
            m_pDEDWindow->SetFlipImageHorizontally(m_bDEDFlippedHorizontically);
            m_pDEDWindow->SetUseDefaultWidth(m_bDEDDefaultSize_W);
            m_pDEDWindow->SetUseDefaultHeight(m_bDEDDefaultSize_H);

            m_pDEDWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pDEDWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pDEDWindow->IsClosed())
                m_pDEDWindow->ShowWindow();

            m_pDEDWindow->MoveWindow(m_iDED_X, m_iDED_Y, m_iDED_W, m_iDED_H, m_bDED_ONTOP);
            m_pDEDWindow->Render(m_pSharedMemory->Connected(), m_pSharedMemory->DisplayImage(DED));
        }
    }

    if (m_bUseRWR)
    {
        if (m_pRWRWindow == nullptr)
        {
            m_pRWRWindow = new CDisplayWindow("RWR", m_iRWR_X, m_iRWR_Y, m_iRWR_W, m_iRWR_H, m_bRWR_ONTOP);
            m_pRWRWindow->SetBackgroundImage(m_sRWRBackgroundImage);

            m_pRWRWindow->SetFlipImageVertically(m_bRWRFlippedVertically);
            m_pRWRWindow->SetFlipImageHorizontally(m_bRWRFlippedHorizontically);
            m_pRWRWindow->SetUseDefaultWidth(m_bRWRDefaultSize_W);
            m_pRWRWindow->SetUseDefaultHeight(m_bRWRDefaultSize_H);

            m_pRWRWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pRWRWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pRWRWindow->IsClosed())
                m_pRWRWindow->ShowWindow();

            m_pRWRWindow->MoveWindow(m_iRWR_X, m_iRWR_Y, m_iRWR_W, m_iRWR_H, m_bRWR_ONTOP);
            m_pRWRWindow->Render(m_pSharedMemory->Connected(), m_pSharedMemory->DisplayImage(RWR));
        }
    }

    if (m_bUseMFDLEFT)
    {
        if (m_pMFDLEFTWindow == nullptr)
        {
            m_pMFDLEFTWindow = new CDisplayWindow("LEFT MFD", m_iMFDLEFT_X, m_iMFDLEFT_Y, m_iMFDLEFT_W, m_iMFDLEFT_H, m_bMFDLEFT_ONTOP);
            m_pMFDLEFTWindow->SetBackgroundImage(m_sMFDLEFTBackgroundImage);

            m_pMFDLEFTWindow->SetFlipImageVertically(m_bMFDLEFTFlippedVertically);
            m_pMFDLEFTWindow->SetFlipImageHorizontally(m_bMFDLEFTFlippedHorizontically);
            m_pMFDLEFTWindow->SetUseDefaultWidth(m_bMFDLEFTDefaultSize_W);
            m_pMFDLEFTWindow->SetUseDefaultHeight(m_bMFDLEFTDefaultSize_H);

            m_pMFDLEFTWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pMFDLEFTWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pMFDLEFTWindow->IsClosed())
                m_pMFDLEFTWindow->ShowWindow();

            m_pMFDLEFTWindow->MoveWindow(m_iMFDLEFT_X, m_iMFDLEFT_Y, m_iMFDLEFT_W, m_iMFDLEFT_H, m_bMFDLEFT_ONTOP);
            m_pMFDLEFTWindow->Render(m_pSharedMemory->Connected(), m_pSharedMemory->DisplayImage(MFDLEFT));
        }
    }

    if (m_bUseMFDRIGHT)
    {
        if (m_pMFDRIGHTWindow == nullptr)
        {
            m_pMFDRIGHTWindow = new CDisplayWindow("RIGHT MFD", m_iMFDRIGHT_X, m_iMFDRIGHT_Y, m_iMFDRIGHT_W, m_iMFDRIGHT_H, m_bMFDRIGHT_ONTOP);
            m_pMFDRIGHTWindow->SetBackgroundImage(m_sMFDRIGHTBackgroundImage);

            m_pMFDRIGHTWindow->SetFlipImageVertically(m_bMFDRIGHTFlippedVertically);
            m_pMFDRIGHTWindow->SetFlipImageHorizontally(m_bMFDRIGHTFlippedHorizontically);
            m_pMFDRIGHTWindow->SetUseDefaultWidth(m_bMFDRIGHTDefaultSize_W);
            m_pMFDRIGHTWindow->SetUseDefaultHeight(m_bMFDRIGHTDefaultSize_H);

            m_pMFDRIGHTWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pMFDRIGHTWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pMFDRIGHTWindow->IsClosed())
                m_pMFDRIGHTWindow->ShowWindow();

            m_pMFDRIGHTWindow->MoveWindow(m_iMFDRIGHT_X, m_iMFDRIGHT_Y, m_iMFDRIGHT_W, m_iMFDRIGHT_H, m_bMFDRIGHT_ONTOP);
            m_pMFDRIGHTWindow->Render(m_pSharedMemory->Connected(), m_pSharedMemory->DisplayImage(MFDRIGHT));
        }
    }

    if (m_bUseHMS)
    {
        if (m_pHMSWindow == nullptr)
        {
            m_pHMSWindow = new CDisplayWindow("HMS", m_iHMS_X, m_iHMS_Y, m_iHMS_W, m_iHMS_H, m_bHMS_ONTOP);
            m_pHMSWindow->SetBackgroundImage(m_sHMSBackgroundImage);

            m_pHMSWindow->SetFlipImageVertically(m_bHMSFlippedVertically);
            m_pHMSWindow->SetFlipImageHorizontally(m_bHMSFlippedHorizontically);
            m_pHMSWindow->SetUseDefaultWidth(m_bHMSDefaultSize_W);
            m_pHMSWindow->SetUseDefaultHeight(m_bHMSDefaultSize_H);

            m_pHMSWindow->SetShowPositionInfo(m_bShowPositionInfo);
            m_pHMSWindow->SetWindowMovable(m_bWindowsMovable);
        }
        else
        {
            if (m_pHMSWindow->IsClosed())
                m_pHMSWindow->ShowWindow();

            m_pHMSWindow->MoveWindow(m_iHMS_X, m_iHMS_Y, m_iHMS_W, m_iHMS_H, m_bHMS_ONTOP);
            m_pHMSWindow->Render(m_pSharedMemory->Connected(), m_pSharedMemory->DisplayImage(HMS));
        }
    }
}

void Application::CloseDisplays()
{
    LOG_DEBUG("Application::CloseDisplays() Begin");

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

    LOG_DEBUG("Application::CloseDisplays() End");
}

void Application::ReadSettings()
{
    LOG_DEBUG("Application::ReadSettings() Begin");

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

    m_pConfigReader->getValue("SHOW_POSITION_INFO", m_bShowPositionInfo);
    CLogger::getInstance()->debug("   ShowPositionInfo : %s", m_bShowPositionInfo ? "true" : "false");

    m_pConfigReader->getValue("WINDOWS_MOVABLE", m_bWindowsMovable);
    CLogger::getInstance()->debug("   WindowsMovable : %s", m_bWindowsMovable ? "true" : "false");

    m_pConfigReader->getValue("HUD_BACKGROUND_IMAGE", m_sHUDBackgroundImage);
    CLogger::getInstance()->debug("   HUD_BACKGROUND_IMAGE : %s", m_sHUDBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_HUD", m_bUseHUD);
    CLogger::getInstance()->debug("   HUD : %s", m_bUseHUD ? "true" : "false");

    m_pConfigReader->getValue("HUD_FLIPPED_VERTICALLY", m_bHUDFlippedVertically);
    CLogger::getInstance()->debug("   HUD Flipped Vertically : %s", m_bHUDFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("HUD_FLIPPED_HORIZONTICALLY", m_bHUDFlippedHorizontically);
    CLogger::getInstance()->debug("   HUD Flipped Horizontically : %s", m_bHUDFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("HUD_X", m_iHUD_X);
    CLogger::getInstance()->debug("   HUD X : %d", m_iHUD_X);

    m_pConfigReader->getValue("HUD_Y", m_iHUD_Y);
    CLogger::getInstance()->debug("   HUD Y : %d", m_iHUD_Y);

    m_pConfigReader->getValue("HUD_W", m_iHUD_W);
    CLogger::getInstance()->debug("   HUD W : %d", m_iHUD_W);
    if (m_iHUD_W <= 0)
    {
        m_bHUDDefaultSize_W = true;
        m_iHUD_W = 600;
    }

    m_pConfigReader->getValue("HUD_H", m_iHUD_H);
    CLogger::getInstance()->debug("   HUD H : %d", m_iHUD_H);
    if (m_iHUD_H <= 0)
    {
        m_bHUDDefaultSize_H = true;
        m_iHUD_H = 600;
    }

    m_pConfigReader->getValue("HUD_ONTOP", m_bHUD_ONTOP);
    CLogger::getInstance()->debug("   HUD ONTOP : %s", m_bHUD_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("PFL_FLIPPED_VERTICALLY", m_bPFLFlippedVertically);
    CLogger::getInstance()->debug("   PFL Flipped Vertically : %s", m_bPFLFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("PFL_FLIPPED_HORIZONTICALLY", m_bPFLFlippedHorizontically);
    CLogger::getInstance()->debug("   PFL Flipped Horizontically : %s", m_bPFLFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("PFL_BACKGROUND_IMAGE", m_sPFLBackgroundImage);
    CLogger::getInstance()->debug("   PFL_BACKGROUND_IMAGE : %s", m_sPFLBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_PFL", m_bUsePFL);
    CLogger::getInstance()->debug("   PFL : %s", m_bUsePFL ? "true" : "false");

    m_pConfigReader->getValue("PFL_X", m_iPFL_X);
    CLogger::getInstance()->debug("   PFL X : %d", m_iPFL_X);

    m_pConfigReader->getValue("PFL_Y", m_iPFL_Y);
    CLogger::getInstance()->debug("   PFL Y : %d", m_iPFL_Y);

    m_pConfigReader->getValue("PFL_W", m_iPFL_W);
    CLogger::getInstance()->debug("   PFL W : %d", m_iPFL_W);
    if (m_iPFL_W <= 0)
    {
        m_bPFLDefaultSize_W = true;
        m_iPFL_W = 600;
    }

    m_pConfigReader->getValue("PFL_H", m_iPFL_H);
    CLogger::getInstance()->debug("   PFL H : %d", m_iPFL_H);
    if (m_iPFL_H <= 0)
    {
        m_bPFLDefaultSize_H = true;
        m_iPFL_H = 600;
    }

    m_pConfigReader->getValue("PFL_ONTOP", m_bPFL_ONTOP);
    CLogger::getInstance()->debug("   PFL ONTOP : %s", m_bPFL_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("DED_BACKGROUND_IMAGE", m_sDEDBackgroundImage);
    CLogger::getInstance()->debug("   DED_BACKGROUND_IMAGE : %s", m_sDEDBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_DED", m_bUseDED);
    CLogger::getInstance()->debug("   DED : %s", m_bUseDED ? "true" : "false");

    m_pConfigReader->getValue("DED_FLIPPED_VERTICALLY", m_bDEDFlippedVertically);
    CLogger::getInstance()->debug("   DED Flipped Vertically : %s", m_bDEDFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("DED_FLIPPED_HORIZONTICALLY", m_bDEDFlippedHorizontically);
    CLogger::getInstance()->debug("   DED Flipped Horizontically : %s", m_bDEDFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("DED_X", m_iDED_X);
    CLogger::getInstance()->debug("   DED X : %d", m_iDED_X);

    m_pConfigReader->getValue("DED_Y", m_iDED_Y);
    CLogger::getInstance()->debug("   DED Y : %d", m_iDED_Y);

    m_pConfigReader->getValue("DED_W", m_iDED_W);
    CLogger::getInstance()->debug("   DED W : %d", m_iDED_W);
    if (m_iDED_W <= 0)
    {
        m_bDEDDefaultSize_W = true;
        m_iDED_W = 600;
    }

    m_pConfigReader->getValue("DED_H", m_iDED_H);
    CLogger::getInstance()->debug("   DED H : %d", m_iDED_H);
    if (m_iDED_H <= 0)
    {
        m_bDEDDefaultSize_H = true;
        m_iDED_H = 600;
    }

    m_pConfigReader->getValue("DED_ONTOP", m_bDED_ONTOP);
    CLogger::getInstance()->debug("   DED ONTOP : %s", m_bDED_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("RWR_BACKGROUND_IMAGE", m_sRWRBackgroundImage);
    CLogger::getInstance()->debug("   RWR_BACKGROUND_IMAGE : %s", m_sRWRBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_RWR", m_bUseRWR);
    CLogger::getInstance()->debug("   RWR : %s", m_bUseRWR ? "true" : "false");

    m_pConfigReader->getValue("RWR_FLIPPED_VERTICALLY", m_bRWRFlippedVertically);
    CLogger::getInstance()->debug("   RWR Flipped Vertically : %s", m_bRWRFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("RWR_FLIPPED_HORIZONTICALLY", m_bRWRFlippedHorizontically);
    CLogger::getInstance()->debug("   RWR Flipped Horizontically : %s", m_bRWRFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("RWR_X", m_iRWR_X);
    CLogger::getInstance()->debug("   RWR X : %d", m_iRWR_X);

    m_pConfigReader->getValue("RWR_Y", m_iRWR_Y);
    CLogger::getInstance()->debug("   RWR Y : %d", m_iRWR_Y);

    m_pConfigReader->getValue("RWR_W", m_iRWR_W);
    CLogger::getInstance()->debug("   RWR W : %d", m_iRWR_W);
    if (m_iRWR_W <= 0)
    {
        m_bRWRDefaultSize_W = true;
        m_iRWR_W = 600;
    }

    m_pConfigReader->getValue("RWR_H", m_iRWR_H);
    CLogger::getInstance()->debug("   RWR H : %d", m_iRWR_H);
    if (m_iRWR_H <= 0)
    {
        m_bRWRDefaultSize_H = true;
        m_iRWR_H = 600;
    }

    m_pConfigReader->getValue("RWR_ONTOP", m_bRWR_ONTOP);
    CLogger::getInstance()->debug("   RWR ONTOP : %s", m_bRWR_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("MFDLEFT_BACKGROUND_IMAGE", m_sMFDLEFTBackgroundImage);
    CLogger::getInstance()->debug("   MFDLEFT_BACKGROUND_IMAGE : %s", m_sMFDLEFTBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_MFDLEFT", m_bUseMFDLEFT);
    CLogger::getInstance()->debug("   LEFT MFD : %s", m_bUseMFDLEFT ? "true" : "false");

    m_pConfigReader->getValue("MFDLEFT_FLIPPED_VERTICALLY", m_bMFDLEFTFlippedVertically);
    CLogger::getInstance()->debug("   MFDLEFT Flipped Vertically : %s", m_bMFDLEFTFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("MFDLEFT_FLIPPED_HORIZONTICALLY", m_bMFDLEFTFlippedHorizontically);
    CLogger::getInstance()->debug("   MFDLEFT Flipped Horizontically : %s", m_bMFDLEFTFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("MFDLEFT_X", m_iMFDLEFT_X);
    CLogger::getInstance()->debug("   LEFT MFD X : %d", m_iMFDLEFT_X);

    m_pConfigReader->getValue("MFDLEFT_Y", m_iMFDLEFT_Y);
    CLogger::getInstance()->debug("   LEFT MFD Y : %d", m_iMFDLEFT_Y);

    m_pConfigReader->getValue("MFDLEFT_W", m_iMFDLEFT_W);
    CLogger::getInstance()->debug("   LEFT MFD W : %d", m_iMFDLEFT_W);
    if (m_iMFDLEFT_W <= 0)
    {
        m_bMFDLEFTDefaultSize_W = true;
        m_iMFDLEFT_W = 600;
    }

    m_pConfigReader->getValue("MFDLEFT_H", m_iMFDLEFT_H);
    CLogger::getInstance()->debug("   LEFT MFD H : %d", m_iMFDLEFT_H);
    if (m_iMFDLEFT_H <= 0)
    {
        m_bMFDLEFTDefaultSize_H = true;
        m_iMFDLEFT_H = 600;
    }

    m_pConfigReader->getValue("MFDLEFT_ONTOP", m_bMFDLEFT_ONTOP);
    CLogger::getInstance()->debug("   LEFT MFD ONTOP : %s", m_bMFDLEFT_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("MFDRIGHT_BACKGROUND_IMAGE", m_sMFDRIGHTBackgroundImage);
    CLogger::getInstance()->debug("   MFDRIGHT_BACKGROUND_IMAGE : %s", m_sMFDRIGHTBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_MFDRIGHT", m_bUseMFDRIGHT);
    CLogger::getInstance()->debug("   RIGHT MFD : %s", m_bUseMFDRIGHT ? "true" : "false");

    m_pConfigReader->getValue("MFDRIGHT_FLIPPED_VERTICALLY", m_bMFDRIGHTFlippedVertically);
    CLogger::getInstance()->debug("   MFDRIGHT Flipped Vertically : %s", m_bMFDRIGHTFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("MFDRIGHT_FLIPPED_HORIZONTICALLY", m_bMFDRIGHTFlippedHorizontically);
    CLogger::getInstance()->debug("   MFDRIGHT Flipped Horizontically : %s", m_bMFDRIGHTFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("MFDRIGHT_X", m_iMFDRIGHT_X);
    CLogger::getInstance()->debug("   RIGHT MFD X : %d", m_iMFDRIGHT_X);

    m_pConfigReader->getValue("MFDLEFT_Y", m_iMFDRIGHT_Y);
    CLogger::getInstance()->debug("   RIGHT MFD Y : %d", m_iMFDRIGHT_Y);

    m_pConfigReader->getValue("MFDLEFT_W", m_iMFDRIGHT_W);
    CLogger::getInstance()->debug("   RIGHT MFD W : %d", m_iMFDRIGHT_W);
    if (m_iMFDRIGHT_W <= 0)
    {
        m_bMFDRIGHTDefaultSize_W = true;
        m_iMFDRIGHT_W = 800;
    }

    m_pConfigReader->getValue("MFDLEFT_H", m_iMFDRIGHT_H);
    CLogger::getInstance()->debug("   RIGHT MFD H : %d", m_iMFDRIGHT_H);
    if (m_iMFDRIGHT_H <= 0)
    {
        m_bMFDRIGHTDefaultSize_H = true;
        m_iMFDRIGHT_H = 600;
    }

    m_pConfigReader->getValue("MFDLEFT_ONTOP", m_bMFDRIGHT_ONTOP);
    CLogger::getInstance()->debug("   RIGHT MFD ONTOP : %s", m_bMFDRIGHT_ONTOP ? "true" : "false");

    m_pConfigReader->getValue("HMS_BACKGROUND_IMAGE", m_sHMSBackgroundImage);
    CLogger::getInstance()->debug("   HMS_BACKGROUND_IMAGE : %s", m_sHMSBackgroundImage.c_str());

    m_pConfigReader->getValue("USE_HMS", m_bUseHMS);
    CLogger::getInstance()->debug("   HMS : %s", m_bUseHMS ? "true" : "false");
    
    m_pConfigReader->getValue("HMS_FLIPPED_VERTICALLY", m_bHMSFlippedVertically);
    CLogger::getInstance()->debug("   HMS Flipped Vertically : %s", m_bHMSFlippedVertically ? "true" : "false");

    m_pConfigReader->getValue("HMS_FLIPPED_HORIZONTICALLY", m_bHMSFlippedHorizontically);
    CLogger::getInstance()->debug("   HMS Flipped Horizontically : %s", m_bHMSFlippedHorizontically ? "true" : "false");

    m_pConfigReader->getValue("HMS_X", m_iHMS_X);
    CLogger::getInstance()->debug("   HMS X : %d", m_iHMS_X);

    m_pConfigReader->getValue("HMS_Y", m_iHMS_Y);
    CLogger::getInstance()->debug("   HMS Y : %d", m_iHMS_Y);

    m_pConfigReader->getValue("HMS_W", m_iHMS_W);
    CLogger::getInstance()->debug("   HMS W : %d", m_iHMS_W);
    if (m_iHMS_W <= 0)
    {
        m_bHMSDefaultSize_W = true;
        m_iHMS_W = 600;
    }

    m_pConfigReader->getValue("HMS_H", m_iHMS_H);
    CLogger::getInstance()->debug("   HMS H : %d", m_iHMS_H);
    if (m_iHMS_H <= 0)
    {
        m_bHMSDefaultSize_H = true;
        m_iHMS_H = 600;
    }

    m_pConfigReader->getValue("HMS_ONTOP", m_bHMS_ONTOP);
    CLogger::getInstance()->debug("   HMS ONTOP : %s", m_bHMS_ONTOP ? "true" : "false");

    LOG_DEBUG("Application::ReadSettings() End");
}