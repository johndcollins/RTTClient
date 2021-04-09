// RTTClient.cpp : Defines the entry point for the application.
//

#include "CConfigReader.h"
#include "CRTTClient.h"
#include "CLogger.h"

// opengl
#include <GL/glew.h>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#include <fstream>
#include <vector>

using namespace std;
using namespace RTTClient::Common;

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 600

int main()
{
    CLogger::getInstance()->updateLogLevel(LogLevel::LOG_LEVEL_DEBUG);

    CConfigReader* p = CConfigReader::getInstance();

    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string path;
    if (count != -1)
        path = dirname(result);
    else
    {
        LOG_ERROR("Startup : Unable to locate exe directory");
        return -1;
    }

    path.append("/RTTClient.ini");

    ifstream f(path.c_str());
    if (!f.good())
    {
        CLogger::getInstance()->error("Startup : Unable to locate RTTClient.ini file. Path : %s", path);
        return -1;
    }

    // parse the configuration file
    p->parseFile(path);

    SDL_Init(0);

    LOG_DEBUG("Testing Video Drivers...");
    std::vector< bool > drivers(SDL_GetNumVideoDrivers());
    for (int i = 0; i < drivers.size(); ++i)
    {
        drivers[i] = (0 == SDL_VideoInit(SDL_GetVideoDriver(i)));
        SDL_VideoQuit();
    }

    LOG_DEBUG("SDL_VIDEODRIVER available:");
    for (int i = 0; i < drivers.size(); ++i)
    {
        CLogger::getInstance()->debug("  %s", SDL_GetVideoDriver(i));
    }
    LOG_DEBUG("");

    LOG_DEBUG("SDL_VIDEODRIVER usable:");
    for (int i = 0; i < drivers.size(); ++i)
    {
        if (!drivers[i]) continue;
        CLogger::getInstance()->debug("  %s", SDL_GetVideoDriver(i));
    }
    LOG_DEBUG("");

    // ----- Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        LOG_ERROR("SDL could not initialize.");
        CLogger::getInstance()->error("SDL could not initialize. %s", SDL_GetError());
        return -1;
    }
    else
        LOG_DEBUG("SDL Initialized.");

    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG))
    {
        CLogger::getInstance()->error("Error, while intitializing IMG PNG and IMG JPG. %s", IMG_GetError());
        return -1;
    }
    else
        LOG_DEBUG("IMG Initialized.");

    if (TTF_Init() == -1)
    {
        CLogger::getInstance()->error("Failed to initialize SDL_TTF. %s", TTF_GetError());
        return -1;
    }
    else
        LOG_DEBUG("TTF Initialized.");

    CLogger::getInstance()->debug("SDL_VIDEODRIVER selected : %s", SDL_GetCurrentVideoDriver());

    // ----- Create window
    SDL_Window* window = SDL_CreateWindow("RTTClient", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE_X, SCREEN_SIZE_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SKIP_TASKBAR /*| SDL_WINDOW_OPENGL*/ | SDL_WINDOW_SHOWN);
    if (!window)
    {
        CLogger::getInstance()->error("SDL Error creating window. %s", SDL_GetError());
        return -1;
    }
    else
        LOG_DEBUG("SDL Window created.");
    
    LOG_DEBUG("SDL_RENDER_DRIVER available:");
    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);
        CLogger::getInstance()->debug("  %s", info.name);
    }
    LOG_DEBUG("");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        CLogger::getInstance()->error("SDL Error creating renderer. %s", SDL_GetError());
        return -1;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    CLogger::getInstance()->debug("SDL_RENDER_DRIVER selected : %s", info.name);

    // ----- SDL OpenGL settings
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // ----- SDL OpenGL context
    //SDL_GLContext glContext = SDL_GL_CreateContext(window);

    // ----- SDL v-sync
    //SDL_GL_SetSwapInterval(1);

    // ----- GLEW
    //glewInit();
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    bool value = false;
    p->getValue("USE_MFDLEFT", value);
    CLogger::getInstance()->debug("LEFT MFD : %s", value ? "true" : "false");

    p->getValue("USE_MFDRIGHT", value);
    CLogger::getInstance()->debug("RIGHT MFD : %s", value ? "true" : "false");

    p->getValue("USE_HUD", value);
    CLogger::getInstance()->debug("HUD : %s", value ? "true" : "false");

    string ipAddr("");
    p->getValue("HOST", ipAddr);
    CLogger::getInstance()->debug("HOST : %s", ipAddr.c_str());

    int port(0);
    p->getValue("PORT", port);
    CLogger::getInstance()->debug("PORT : %d", port);

    CNetworkManager networkMgr = CNetworkManager();
    if (!networkMgr.Start())
        LOG_DEBUG("Startup : Network Start Failed");
    else
        LOG_DEBUG("Startup : Network Started");

    p->getValue("NETWORKED", value);
    CLogger::getInstance()->debug("Networked : %s", value ? "true" : "false");

    if (value)
    {
        LOG_DEBUG("Connecting...");
        networkMgr.Connect(ipAddr.c_str(), port);
        if (networkMgr.g_ConnectionState == CONSTATE_COND)
            LOG_DEBUG("Connected");
        else if (networkMgr.g_ConnectionState == CONSTATE_DISC)
            LOG_DEBUG("Disconnected");
        else if (networkMgr.g_ConnectionState == CONSTATE_FAIL)
            LOG_DEBUG("Conenction failed");
    }

    // ----- App loop
    bool quit = false;
    while (quit == false)
    {
        SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent))
        {
            switch (windowEvent.type)
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
                quit = true;
                break;
            case SDL_WINDOWEVENT:
                //if (e.window.event == SDL_WINDOWEVENT_MINIMIZED)
                //    SDL_HideWindow(window);
                break;
            }
        }

        // DisplayWindow Updates
        if (networkMgr.g_ConnectionState == CONSTATE_DISC && networkMgr.g_ConnectionState == CONSTATE_FAIL)
            networkMgr.Connect(ipAddr.c_str(), port);

        networkMgr.Pulse();

        SDL_RenderClear(renderer);

        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = SCREEN_SIZE_X;
        rect.h = SCREEN_SIZE_Y;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_RenderPresent(renderer);


        //SDL_GL_MakeCurrent(window, glContext);
        //SDL_GL_SwapWindow(window);
    }

    LOG_DEBUG("Shutting Down...");

    networkMgr.Disconnect();

    //SDL_GL_DeleteContext(glContext);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
