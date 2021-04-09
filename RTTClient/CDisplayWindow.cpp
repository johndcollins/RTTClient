#include "CDisplayWindow.h"

#include <GL/glew.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "CLogger.h"

using namespace std;
using namespace RTTClient::Common;

CDisplayWindow::CDisplayWindow(const std::string& title, int x, int y, int width, int height, bool ontop) :
    m_sTitle(title), m_iWindow_X(x), m_iWindow_Y(y), m_iWindow_W(width), m_iWindow_H(height), m_bWindow_Ontop(ontop)
{
    LOG_DEBUG("CDisplayWindow::CDisplayWindow() Begin");
    m_bWindow_Created = !Init();
    LOG_DEBUG("CDisplayWindow::CDisplayWindow() End");
}

CDisplayWindow::~CDisplayWindow()
{
    LOG_DEBUG("CDisplayWindow::~CDisplayWindow() Begin");
    SDL_DestroyRenderer(m_pWindowRenderer);
    //SDL_GL_DeleteContext(m_pGLContext);
    SDL_DestroyWindow(m_pWindow);
    m_pWindow = nullptr;
    LOG_DEBUG("CDisplayWindow::~CDisplayWindow() End");
}

bool CDisplayWindow::Init()
{
    LOG_DEBUG("CDisplayWindow::Init() Begin");

    //CLogger::getInstance()->debug("Creating window. Title %s, X %d, Y %d, W %d, H %d, ONTOP %s", m_sTitle, m_iWindow_X, m_iWindow_Y, m_iWindow_W, m__iWindow_H, m_bWindow_Ontop ? "true" : "false");
    
    // ----- Create window
    m_pWindow = SDL_CreateWindow(m_sTitle.c_str(), m_iWindow_X, m_iWindow_Y, m_iWindow_W, m_iWindow_H, SDL_WINDOW_RESIZABLE | /*SDL_WINDOW_OPENGL |*/ SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (!m_pWindow)
    {
        CLogger::getInstance()->error("CDisplayWindow::Init() Error creating window. %s", SDL_GetError());
        return false;
    }

    m_bWindowShown = true;
    m_iWindowID = SDL_GetWindowID(m_pWindow);
    CLogger::getInstance()->debug("CDisplayWindow::Init() Window Id : %d", m_iWindowID);

    // ----- SDL OpenGL settings
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // ----- SDL OpenGL context
    //m_pGLContext = SDL_GL_CreateContext(m_pWindow);

    // ----- SDL v-sync
    //SDL_GL_SetSwapInterval(1);

    // ----- GLEW
    //glewInit();
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindowRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_pWindowRenderer)
    {
        LOG_ERROR("ApplicatCDisplayWindow::Init() Failed to get window's surface");
        CLogger::getInstance()->error("CDisplayWindow::Init() SDL2 Error: %s", SDL_GetError());
        return false;
    }

    LOG_DEBUG("CDisplayWindow::Init() End");

    return true;
}

void CDisplayWindow::Render()
{

    SDL_RenderClear(m_pWindowRenderer);
    
    if (m_bWindowShown)
    {

    }
    
    SDL_RenderPresent(m_pWindowRenderer);

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

    //SDL_GL_SwapWindow(m_pWindow);
}

bool CDisplayWindow::HandleEvents(SDL_Event& event)
{
    if (event.type == SDL_WINDOWEVENT && event.window.windowID == m_iWindowID)
    {
        switch (event.window.event)
        {
            //Window appeared
        case SDL_WINDOWEVENT_SHOWN:
            m_bWindowShown = true;
            break;
            //Window disappeared
        case SDL_WINDOWEVENT_HIDDEN:
            m_bWindowShown = false;
            break;
            //Get new dimensions and repaint
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            m_iWindow_W = m_windowEvent.window.data1;
            m_iWindow_H = m_windowEvent.window.data2;
            SDL_RenderPresent(m_pWindowRenderer);
            break;
            //Repaint on expose
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_RenderPresent(m_pWindowRenderer);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            LOG_DEBUG("CDisplayWindow::HandleEvents() SDL_WINDOWEVENT_CLOSE");
            CloseWindow();
        break;
        default: break;
        }

        return true;
    }
    else
        return false;
}

void CDisplayWindow::ShowWindow()
{
    //Restore window if needed
    if (!m_bWindowShown)
    {
        SDL_ShowWindow(m_pWindow);
    }

    //Move window forward
    SDL_RaiseWindow(m_pWindow);
}

void CDisplayWindow::MoveWindow(int x, int y, int width, int height, bool ontop)
{
    SDL_GetWindowPosition(m_pWindow, &x, &y);
    SDL_GetWindowSize(m_pWindow, &width, &height);
}

void CDisplayWindow::CloseWindow()
{
    SDL_HideWindow(m_pWindow);
    m_bWindowShown = false;
}

void CDisplayWindow::Clear() const
{

    //Present first to display sprites before rendering background
    SDL_RenderPresent(m_pWindowRenderer);

    //Set the color to the render - R G B A
    SDL_SetRenderDrawColor(m_pWindowRenderer, 0, 0, 200, 255);

    SDL_RenderClear(m_pWindowRenderer);
}