#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 

#include "misc/dr4_ifc.hpp"
#include "dr4/window.hpp"
#include "dr4/texture.hpp"

#include "Window.hpp"
#include "Drawable.hpp"

namespace ia {
    
class IAGraphicsBackEnd : public dr4::DR4Backend {
    const std::string name_ = "IAGraphicsBackEnd";

public:
    IAGraphicsBackEnd() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("IAGraphicsBackEnd : SDL_Init Error. %s\n", SDL_GetError());
            SDL_Quit();
            return;
        }

        if (TTF_Init() < 0) {
            printf("Couldn't initialize TTF: %s\n", TTF_GetError());
            SDL_Quit();
            return;
        }

    }
    ~IAGraphicsBackEnd() { SDL_Quit(); }

    const std::string &Name() const { return name_; }

    dr4::Window *CreateWindow() { return new Window("Window"); }
};

}
