#pragma once

#include <string>
#include <iostream>
#include "CTools.h"

#include <SDL2/SDL.h>

class DisplayWindow
{
public:
    DisplayWindow(const std::string& title, int x, int y, int width, int height, bool ontop);
    virtual ~DisplayWindow();

    void update();
    void pollEvents(SDL_Event& event);
    void clear() const;
    inline bool isClosed() const { return _closed; }

    static SDL_Renderer* renderer;

private:
    bool init();
    //SDL_Surface* horizontal_mirror(SDL_Surface* sfc);
    //SDL_Surface* flip_vertically(SDL_Surface* sfc);

    std::string _title;
    int _x = 0;
    int _y = 0;
    int _width = 640;
    int _height = 480;
    bool _ontop = false;
    bool _closed = false;

    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext;
};
