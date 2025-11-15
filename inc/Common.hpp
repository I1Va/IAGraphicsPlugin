#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "dr4/keycodes.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/mouse_buttons.hpp"

namespace ia {

SDL_Color convertToSDLColor(const dr4::Color &color);
dr4::Color convertToDr4Color(const SDL_Color color);

SDL_Rect convertToSDLRect(const dr4::Rect2f &rect);

dr4::KeyMode convertToDr4KeyMode(const Uint16 SDLKeyMod);
dr4::KeyCode convertToDr4KeyCode(const SDL_Keycode SDLKeySym);
dr4::MouseButtonType convertToDr4MouseButton(const Uint8 SDLButton);

inline Uint32 SDLColorToGfxColor(SDL_Color c) {
    return (c.a << 24) | (c.r << 16) | (c.g << 8) | (c.b);
}

inline bool isNullRect(const SDL_Rect &rect) {
    return (rect.x == 0 && rect.y == 0 &&
            rect.w == 0 && rect.h == 0);
}

struct RendererGuard {
    SDL_Renderer* renderer_;
    SDL_Texture* savedTarget_;
    SDL_Rect savedViewport_;
    SDL_Rect savedClip_;
    Uint8 r_, g_, b_, a_;
    SDL_BlendMode blend_;

RendererGuard(SDL_Renderer* renderer) : renderer_(renderer) {
    assert(renderer_);

    savedTarget_ = SDL_GetRenderTarget(renderer_);
    SDL_GetRenderDrawColor(renderer_, &r_, &g_, &b_, &a_);
    SDL_GetRenderDrawBlendMode(renderer_, &blend_);
    SDL_RenderGetViewport(renderer_, &savedViewport_);
    SDL_RenderGetClipRect(renderer_, &savedClip_);
};

~RendererGuard() {
    SDL_SetRenderTarget(renderer_, savedTarget_);

    SDL_SetRenderDrawColor(renderer_, r_, g_, b_, a_);
    SDL_SetRenderDrawBlendMode(renderer_, blend_);

    if (!isNullRect(savedViewport_))
        SDL_RenderSetViewport(renderer_, &savedViewport_);
    else
        SDL_RenderSetViewport(renderer_, nullptr);

    if (!isNullRect(savedClip_))
        SDL_RenderSetClipRect(renderer_, &savedClip_);
    else
        SDL_RenderSetClipRect(renderer_, nullptr);
    }
};

}