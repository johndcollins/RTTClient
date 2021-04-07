// RTTClient.cpp : Defines the entry point for the application.
//

#include "CRTTClient.h"

// opengl
#include <GL/glew.h>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 600

int main()
{
    // ----- Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL could not initialize\n");
        return 1;
    }

    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG))
    {
        std::cerr << "Error, while intitializing IMG PNG and IMG JPG" << std::endl;
        return false;
    }

    if (TTF_Init() == -1)
    {
        std::cerr << "Failed to initialize SDL_TTF" << std::endl;
        return false;
    }

    // ----- Create window
    SDL_Window* window = SDL_CreateWindow("RTTClient", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE_X, SCREEN_SIZE_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "Error creating window.\n");
        return 2;
    }

    // ----- SDL OpenGL settings
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // ----- SDL OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    // ----- SDL v-sync
    SDL_GL_SetSwapInterval(1);

    // ----- GLEW
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

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
        SDL_GL_MakeCurrent(window, glContext);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
