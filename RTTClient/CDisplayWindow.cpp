#include "CDisplayWindow.h"

#include <GL/glew.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

SDL_Renderer* DisplayWindow::renderer = nullptr;

DisplayWindow::DisplayWindow(const std::string& title, int x, int y, int width, int height, bool ontop) :
    _title(title), _x(x), _y(y), _width(width), _height(height), _ontop(ontop)
{
    _closed = !init();
}

DisplayWindow::~DisplayWindow()
{
    SDL_DestroyRenderer(renderer);
    SDL_GL_DeleteContext(_glContext);
    SDL_DestroyWindow(_window);
}

bool DisplayWindow::init()
{
     // ----- Create window
    _window = SDL_CreateWindow(_title.c_str(), _x, _y, _width, _height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (!_window)
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
    _glContext = SDL_GL_CreateContext(_window);

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

void DisplayWindow::update()
{
    SDL_GL_MakeCurrent(_window, _glContext);

    /*
        do drawing here
    */
    SDL_RWops* rwop = SDL_RWFromMem(nullptr, 1);
    int isValid = IMG_isJPG(rwop);
    if (!isValid)
    {
        isValid = IMG_isPNG(rwop);
    }

    if (isValid)
    {
        /*
                SDL_RendererFlip flip;
                if (flipImageHorizontally && flipImageVertically)
                    flip = SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL;
                else if (flipImageHorizontally)
                    flip = SDL_FLIP_HORIZONTAL;
                else if (flipImageVertically)
                    flip = SDL_FLIP_VERTICAL;

                SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0, NULL, flip);
         */


        SDL_Surface* image = IMG_Load_RW(rwop, 1);
        if (!image)
            std::cerr << "Failed to load image" << std::endl;
    }

    SDL_GL_SwapWindow(_window);
}

void DisplayWindow::pollEvents(SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_QUIT: _closed = true; break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            Tools::debug("escape key");
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

void DisplayWindow::clear() const
{

    //Present first to display sprites before rendering background
    SDL_RenderPresent(renderer);

    //Set the color to the render - R G B A
    SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);

    SDL_RenderClear(renderer);
}