#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 

#include "misc/dr4_ifc.hpp"
#include "dr4/window.hpp"
#include "dr4/texture.hpp"

#include "IAError.hpp"
#include "Window.hpp"
#include "Drawable.hpp"

namespace ia {
    
class IAGraphicsBackEnd : public dr4::DR4Backend {
    const std::string name_ = "IAGraphicsBackEnd";

public:
    IAGraphicsBackEnd() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            SDL_Quit();
            throw SDLException("IAGraphicsBackEnd : SDL_Init Error. %s\n" + std::string(SDL_GetError()));
        }

        if (TTF_Init() < 0) {
            SDL_Quit();
            throw TTFException("IAGraphicsBackEnd : TTF_Init Error. %s\n" + std::string(TTF_GetError()));
            return;
        }
    }

    ~IAGraphicsBackEnd() { SDL_Quit(); }

    const std::string &Name() const { return name_; }

    dr4::Window *CreateWindow() { return new Window("Window"); }
};

}
