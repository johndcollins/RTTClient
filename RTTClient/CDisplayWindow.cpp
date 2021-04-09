#include "CDisplayWindow.h"

#include <GL/glew.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "CLogger.h"

using namespace std;
using namespace RTTClient::Common;

SDL_Renderer* CDisplayWindow::m_pRenderer = nullptr;

CDisplayWindow::CDisplayWindow(const std::string& title, int x, int y, int width, int height, bool ontop) :
    m_sTitle(title), m_iWindow_X(x), m_iWindow_Y(y), m_iWindow_W(width), m__iWindow_H(height), m_bWindow_Ontop(ontop)
{
    m_bWindow_Closed = !Init();
}

CDisplayWindow::~CDisplayWindow()
{
    SDL_DestroyRenderer(m_pRenderer);
    SDL_GL_DeleteContext(m_pGLContext);
    SDL_DestroyWindow(m_pWindow);
}

bool CDisplayWindow::Init()
{
     // ----- Create window
    m_pWindow = SDL_CreateWindow(m_sTitle.c_str(), m_iWindow_X, m_iWindow_Y, m_iWindow_W, m__iWindow_H, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (!m_pWindow)
    {
        CLogger::getInstance()->error("::Init() Error creating window. %s", SDL_GetError());
        return false;
    }

    // ----- SDL OpenGL settings
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // ----- SDL OpenGL context
    m_pGLContext = SDL_GL_CreateContext(m_pWindow);

    // ----- SDL v-sync
    SDL_GL_SetSwapInterval(1);

    // ----- GLEW
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void CDisplayWindow::Update()
{
    SDL_GL_MakeCurrent(m_pWindow, m_pGLContext);

    while (SDL_PollEvent(&m_windowEvent) > 0)
    {
        switch (m_windowEvent.type)
        {
            case SDL_QUIT: m_bWindow_Closed = true; break;
            case SDL_KEYDOWN:
                switch (m_windowEvent.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        break;
                }
                break;
                /*
                Mouse case for example
                case SDL_MOUSEMOTION:
                    std::cout << "moving the mouse (" << event.motion.x << ", " << event.motion.y << ")\n";
                    break;
                */
            default: break;
        }
    }

    /*
        do drawing here
    */
    //SDL_RWops* rwop = SDL_RWFromMem(nullptr, 1);
    //int isValid = IMG_isJPG(rwop);
    //if (!isValid)
    //{
    //    isValid = IMG_isPNG(rwop);
    //}

    //if (isValid)
    //{
    //    /*
    //            SDL_RendererFlip flip;
    //            if (flipImageHorizontally && flipImageVertically)
    //                flip = SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL;
    //            else if (flipImageHorizontally)
    //                flip = SDL_FLIP_HORIZONTAL;
    //            else if (flipImageVertically)
    //                flip = SDL_FLIP_VERTICAL;

    //            SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0, NULL, flip);
    //     */


    //    SDL_Surface* image = IMG_Load_RW(rwop, 1);
    //    if (!image)
    //        std::cerr << "Failed to load image" << std::endl;
    //}

    SDL_GL_SwapWindow(m_pWindow);
}

void CDisplayWindow::ShowWindow()
{
}

void CDisplayWindow::MoveWindow(int x, int y, int width, int height, bool ontop)
{
}

void CDisplayWindow::Clear() const
{

    //Present first to display sprites before rendering background
    SDL_RenderPresent(m_pRenderer);

    //Set the color to the render - R G B A
    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 200, 255);

    SDL_RenderClear(m_pRenderer);
}