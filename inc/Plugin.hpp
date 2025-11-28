#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 

#include "cum/ifc/dr4.hpp"
#include "dr4/window.hpp"
#include "dr4/texture.hpp"

#include "IAError.hpp"
#include "Window.hpp"
#include "Drawable.hpp"

namespace ia {
    
struct IAGraphicsBackEnd : public cum::DR4BackendPlugin {
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

    ~IAGraphicsBackEnd() { SDL_Quit(); TTF_Quit(); }

    std::string_view GetIdentifier() const override { return "ru.IAIndustries.dr4BackendProject.IAGraphicsPlugin"; }
    std::string_view GetName() const override { return "IAGraphicsPlugin"; }
    std::string_view GetDescription() const override { 
        return "IAGraphicsPlugin - a wrapper over SDL2 that complies with the dr4::draft2 standard";
    }
    std::vector<std::string_view> GetDependencies() const override  { return {}; }
    std::vector<std::string_view> GetConflicts() const override { return {}; }
    void AfterLoad() override {}

    dr4::Window *CreateWindow() { return new Window("Window"); }
};

}
