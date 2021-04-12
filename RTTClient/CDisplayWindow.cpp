#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#include <vector>
#include <iterator>

#include "CDisplayWindow.h"
#include "SDL2_Gfx/SDL2_gfxPrimitives.h"

#include <GL/glew.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "CLogger.h"

using namespace std;
using namespace RTTClient::Common;

CDisplayWindow::CDisplayWindow(const string& title, int x, int y, int width, int height, bool ontop) :
    m_sTitle(title), m_iWindow_X(x), m_iWindow_Y(y), m_iWindow_W(width), m_iWindow_H(height), m_bWindow_Ontop(ontop)
{
    LOG_DEBUG("CDisplayWindow::CDisplayWindow() Begin");
    m_bWindow_Created = !Init();
    LOG_DEBUG("CDisplayWindow::CDisplayWindow() End");
}

CDisplayWindow::~CDisplayWindow()
{
    LOG_DEBUG("CDisplayWindow::~CDisplayWindow() Begin");
    if (m_pFont != nullptr)
        TTF_CloseFont(m_pFont);
    m_pFont = nullptr;

    SDL_DestroyRenderer(m_pWindowRenderer);
    m_pWindowRenderer = nullptr;
    SDL_DestroyWindow(m_pWindow);
    m_pWindow = nullptr;
    LOG_DEBUG("CDisplayWindow::~CDisplayWindow() End");
}

void CDisplayWindow::SetBackgroundImage(string filename)
{
    if (filename != "")
    {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string path;
        if (count != -1)
            path = dirname(result);
        else
        {
            LOG_ERROR("CDisplayWindow::SetBackgroundImage() Unable to locate exe directory");
            return;
        }

        path.append("/");
        path.append(filename);

        ifstream f(path.c_str());
        if (!f.good())
        {
            CLogger::getInstance()->error("CDisplayWindow::SetBackgroundImage() Unable to locate background image. Path : %s", path);
            return;
        }

        m_sBackgroundImage = filename;
    }
}

bool CDisplayWindow::Init()
{
    LOG_DEBUG("CDisplayWindow::Init() Begin");

    CLogger::getInstance()->debug("Creating window. Title %s, X %d, Y %d, W %d, H %d, ONTOP %s", m_sTitle.c_str(), m_iWindow_X, m_iWindow_Y, m_iWindow_W, m_iWindow_H, m_bWindow_Ontop ? "true" : "false");
    
    // ----- Create window
    m_pWindow = SDL_CreateWindow(m_sTitle.c_str(), m_iWindow_X, m_iWindow_Y, m_iWindow_W, m_iWindow_H, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (!m_pWindow)
    {
        CLogger::getInstance()->error("CDisplayWindow::Init() Error creating window. %s", SDL_GetError());
        return false;
    }

    m_bWindowShown = true;
    m_iWindowID = SDL_GetWindowID(m_pWindow);
    CLogger::getInstance()->debug("CDisplayWindow::Init() Window Id : %d", m_iWindowID);

    m_pWindowRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_pWindowRenderer)
    {
        LOG_ERROR("CDisplayWindow::Init() Failed to get window's surface");
        CLogger::getInstance()->error("CDisplayWindow::Init() SDL2 Error: %s", SDL_GetError());
        return false;
    }

    m_pFont = TTF_OpenFont("font.ttf", 12);
    if (m_pFont == nullptr)
    {
        LOG_ERROR("CDisplayWindow::Init() Failed to setup TTF Font");
        CLogger::getInstance()->error("CDisplayWindow::Init() Failed to load lazy font! SDL_ttf Error: %s", TTF_GetError());
    }

    LOG_DEBUG("CDisplayWindow::Init() End");

    return true;
}

SDL_HitTestResult CDisplayWindow::DraggingCallback(SDL_Window* win, const SDL_Point* area, void* data)
{
    return SDL_HITTEST_DRAGGABLE;
}

void CDisplayWindow::Render(std::vector<unsigned char> image)
{
    if (m_bWindowShown)
    {
        SDL_SetRenderDrawColor(m_pWindowRenderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(m_pWindowRenderer);
        
        SDL_Texture* renderTexture;

        if (!image.empty())
        {
            SDL_RWops* rw = SDL_RWFromMem(&image.front(), image.size());
            if (rw != NULL) {
                SDL_Surface* img = IMG_Load_RW(rw, 1);
                if (img != nullptr)
                {
                    SDL_Surface* tempSurface = SDL_ConvertSurfaceFormat(img, SDL_GetWindowPixelFormat(m_pWindow), 0);
                    if (tempSurface == nullptr)
                        renderTexture = LoadTexture(m_sBackgroundImage);
                    else
                    {
                        renderTexture = SDL_CreateTextureFromSurface(m_pWindowRenderer, tempSurface);
                        SDL_FreeSurface(tempSurface);
                    }

                    SDL_FreeSurface(img);
                }
                else
                    renderTexture = LoadTexture(m_sBackgroundImage);

            }
            else
                renderTexture = LoadTexture(m_sBackgroundImage);;
        }

        if (renderTexture != nullptr)
        {
            if (m_bFlipImageHorizontally || m_bFlipImageVertically)
            {
                SDL_RendererFlip flip;
                if (m_bFlipImageHorizontally && m_bFlipImageVertically)
                    flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
                else if (m_bFlipImageHorizontally)
                    flip = SDL_FLIP_HORIZONTAL;
                else if (m_bFlipImageVertically)
                    flip = SDL_FLIP_VERTICAL;
                else
                    flip = SDL_FLIP_NONE;

                SDL_RenderCopyEx(m_pWindowRenderer, renderTexture, NULL, NULL, 0, NULL, flip);
            }
            else
                SDL_RenderCopy(m_pWindowRenderer, renderTexture, NULL, NULL);
        }
        else
            DrawDefaultBackground();

        if (m_bShowPositionInfo)
            if (m_pFont != nullptr)
                ShowPositionText();

        SDL_DestroyTexture(renderTexture);
        SDL_RenderPresent(m_pWindowRenderer);
    }

}

SDL_Texture* CDisplayWindow::LoadTexture(string path)
{
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL)
    {
        CLogger::getInstance()->error("CDisplayWindow::LoadTexture() Unable to load image %s! SDL_image Error: %s", path, IMG_GetError());
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(m_pWindowRenderer, loadedSurface);
        if (newTexture == NULL)
        {
            CLogger::getInstance()->error("CDisplayWindow::LoadTexture() Unable to create texture from %s! SDL Error: %s", path, SDL_GetError());
        }

        //Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void CDisplayWindow::RenderText(string text, SDL_Rect dest, SDL_Color textColor) {
    SDL_Surface* surf = TTF_RenderText_Solid(m_pFont, text.c_str(), textColor);

    dest.w = surf->w;
    dest.h = surf->h;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_pWindowRenderer, surf);

    SDL_RenderCopy(m_pWindowRenderer, tex, NULL, &dest);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void CDisplayWindow::ShowPositionText()
{
    SDL_Rect dest = { 10, 10, 0, 0 };

    SDL_Color textColor = { 0xFF, 0xFF, 0xFF };
    std::ostringstream stringStream;
    stringStream << "X : " << m_iWindow_X << " Y : " << m_iWindow_Y << " W : " << m_iWindow_W << " H : " << m_iWindow_H;
    string textToDisplay = stringStream.str();
    RenderText(textToDisplay, dest, textColor);
}

void CDisplayWindow::DrawDefaultBackground()
{
    SDL_Rect outlineRect = { 0, 0, m_iWindow_W, m_iWindow_H };
    SDL_SetRenderDrawColor(m_pWindowRenderer, 0xFF, 0x96, 0x00, 0xFF);
    SDL_RenderDrawRect(m_pWindowRenderer, &outlineRect);

    SDL_Rect outlineRect2 = { 1, 1, m_iWindow_W - 1, m_iWindow_H - 1 };
    SDL_RenderDrawRect(m_pWindowRenderer, &outlineRect2);

    aaellipseRGBA(m_pWindowRenderer, m_iWindow_W / 2, m_iWindow_H / 2, m_iWindow_W / 2, m_iWindow_H / 2, 0xFF, 0x96, 0x00, 0xFF);
    aaellipseRGBA(m_pWindowRenderer, m_iWindow_W / 2, m_iWindow_H / 2, m_iWindow_W - 1 / 2, m_iWindow_H - 1 / 2, 0xFF, 0x96, 0x00, 0xFF);


    aalineRGBA(m_pWindowRenderer, 4, 0, m_iWindow_W, m_iWindow_H - 4, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 3, 0, m_iWindow_W, m_iWindow_H - 3, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 2, 0, m_iWindow_W, m_iWindow_H - 2, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 1, 0, m_iWindow_W, m_iWindow_H - 1, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, 0, m_iWindow_W, m_iWindow_H, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, 1, m_iWindow_W - 1, m_iWindow_H, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, 2, m_iWindow_W - 2, m_iWindow_H, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, 3, m_iWindow_W - 3, m_iWindow_H, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, 4, m_iWindow_W - 4, m_iWindow_H, 0xFF, 0x96, 0x00, 0xFF);

    aalineRGBA(m_pWindowRenderer, 0, m_iWindow_H - 4, m_iWindow_W - 4, 0, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, m_iWindow_H - 3, m_iWindow_W - 3, 0, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, m_iWindow_H - 2, m_iWindow_W - 2, 0, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, m_iWindow_H - 1, m_iWindow_W - 1, 0, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 0, m_iWindow_H, m_iWindow_W, 0, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 1, m_iWindow_H, m_iWindow_W, 1, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 2, m_iWindow_H, m_iWindow_W, 2, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 3, m_iWindow_H, m_iWindow_W, 3, 0xFF, 0x96, 0x00, 0xFF);
    aalineRGBA(m_pWindowRenderer, 4, m_iWindow_H, m_iWindow_W, 4, 0xFF, 0x96, 0x00, 0xFF);

    //Draw vertical line of yellow dots
    for (int i = 0; i < m_iWindow_H; i += 4)
    {
        SDL_RenderDrawPoint(m_pWindowRenderer, m_iWindow_W / 2, i);
    }

    //Draw horizontal line of yellow dots
    for (int i = 0; i < m_iWindow_W; i += 4)
    {
        SDL_RenderDrawPoint(m_pWindowRenderer, i, m_iWindow_H / 2);
    }
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
            break;
        case SDL_WINDOWEVENT_MOVED:
            m_iWindow_X = m_windowEvent.window.data1;
            m_iWindow_Y = m_windowEvent.window.data2;
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
    if ((m_iWindow_X != x) || (m_iWindow_Y != y))
    {
        SDL_GetWindowPosition(m_pWindow, &x, &y);
        m_iWindow_X = x;
        m_iWindow_Y = y;
    }

    if ((m_iWindow_W != width) || (m_iWindow_H != height))
    {
        SDL_GetWindowSize(m_pWindow, &width, &height);
        m_iWindow_W = width;
        m_iWindow_H = height;
    }
}

void CDisplayWindow::CloseWindow()
{
    SDL_HideWindow(m_pWindow);
    m_bWindowShown = false;
}

void CDisplayWindow::SetWindowMovable(bool set)
{ 
    m_bWindowsMovable = set; 
    if (m_bWindowsMovable)
    {
        int success = SDL_SetWindowHitTest(m_pWindow, CDisplayWindow::DraggingCallback, NULL);
        if (success == -1)
        {
            LOG_ERROR("CDisplayWindow::SetWindowMovable() Failed to setup DL_SetWindowHitTest");
            CLogger::getInstance()->error("CDisplayWindow::SetWindowMovable() DL_SetWindowHitTest Error: %s", SDL_GetError());
        }

        SDL_SetWindowResizable(m_pWindow, SDL_TRUE);
    }
    else
    {
        int success = SDL_SetWindowHitTest(m_pWindow, NULL, NULL);
        if (success == -1)
        {
            LOG_ERROR("CDisplayWindow::SetWindowMovable() Failed to unset DL_SetWindowHitTest");
            CLogger::getInstance()->error("CDisplayWindow::SetWindowMovable() DL_SetWindowHitTest Error: %s", SDL_GetError());
        }

        SDL_SetWindowResizable(m_pWindow, SDL_FALSE);
    }
}