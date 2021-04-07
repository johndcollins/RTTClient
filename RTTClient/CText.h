#pragma once

#include "SDL2/SDL_ttf.h"
#include "CTools.h"
#include <iostream>

class Text
{
public:
    Text(SDL_Renderer* renderer, const std::string& font_path, int font_size, const std::string& message_text, const SDL_Color& color);
    virtual ~Text();
    void display(int x, int y, SDL_Renderer* renderer) const;
    static SDL_Texture* loadFont(SDL_Renderer* renderer, const std::string& font_path, int font_size, const std::string& message_text, const SDL_Color& color);

protected:

private:
    // Add nullptr as default value
    SDL_Texture* _text_texture = nullptr;
    // As the value can change (non-static), we declare a mutable rect to be able to assign from const member
    mutable SDL_Rect _text_rect;
};