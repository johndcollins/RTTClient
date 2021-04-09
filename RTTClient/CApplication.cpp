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

Application::Application()
{
    CLogger::getInstance()->updateLogLevel(LogLevel::LOG_LEVEL_DEBUG);

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

    bool networked = false;
    m_pConfigReader->getValue("NETWORKED", networked);
    CLogger::getInstance()->debug("Application::Application() Networked : %s", networked ? "true" : "false");
    
    string ipAddress("");
    m_pConfigReader->getValue("HOST", ipAddress);
    CLogger::getInstance()->debug("Application::Application() HOST : %s", ipAddress.c_str());

    int port(0);
    m_pConfigReader->getValue("PORT", port);
    CLogger::getInstance()->debug("Application::Application() PORT : %d", port);

    m_pSharedMemory = new CSharedMemory(networked, ipAddress, port);

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
        return;
    }
    else
        LOG_DEBUG("Application::Application() SDL Initialized.");

    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG))
    {
        CLogger::getInstance()->error("Application::Application() Error, while intitializing IMG PNG and IMG JPG. %s", IMG_GetError());
        return;
    }
    else
        LOG_DEBUG("Application::Application() IMG Initialized.");

    if (TTF_Init() == -1)
    {
        CLogger::getInstance()->error("Application::Application() Failed to initialize SDL_TTF. %s", TTF_GetError());
        return;
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
        return;
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
        return;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(m_pWindowRenderer, &info);
    CLogger::getInstance()->debug("Application::Application() SDL_RENDER_DRIVER selected : %s", info.name);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}


Application::~Application()
{
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
    m_iFrameRate = 30;
    //m_pConfigReader->getValue("FPS", m_iFrameRate);

    //loadbackground();

    bool keep_window_open = true;
    while (keep_window_open)
    {
        while (SDL_PollEvent(&m_windowEvent) > 0)
        {
            switch (m_windowEvent.type)
            {
            case SDL_QUIT:
                keep_window_open = false;
                break;
            }
        }

        update();
        draw();
    }
}

void Application::loadbackground()
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

void Application::drawbackground()
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

void Application::update()
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

void Application::draw()
{
    SDL_RenderClear(m_pWindowRenderer);

    m_iFrameCount++;
    m_iTimerFPS = SDL_GetTicks() - m_iLastFrame;
    if (m_iTimerFPS < (1000 / m_iFrameRate))
    {
        if (m_pImage == NULL)
            drawbackground();
    }

    SDL_RenderPresent(m_pWindowRenderer);
}