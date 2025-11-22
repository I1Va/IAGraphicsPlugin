#pragma once
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "dr4/keycodes.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/mouse_buttons.hpp"

#include "SDLRAII.hpp"
class Window;

namespace ia {

dr4::Rect2f convertToDr4Rect(const SDL_Rect rect);
SDL_Rect convertToSDLRect(const dr4::Rect2f rect);
SDL_Color convertToSDLColor(const dr4::Color color);
dr4::Color convertToDr4Color(const SDL_Color color);

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
    const raii::SDL_Renderer &renderer_;
    raii::SDL_Texture savedTarget_;
    SDL_Rect savedViewport_;
    SDL_Rect savedClip_;
    Uint8 r_, g_, b_, a_;
    SDL_BlendMode blend_;

RendererGuard(const raii::SDL_Renderer &renderer) : renderer_(renderer) {
    assert(renderer_);

    savedTarget_ = raii::SDL_Texture(SDL_GetRenderTarget(renderer_.get()));
    requireSDLCondition(SDL_GetRenderDrawColor(renderer_.get(), &r_, &g_, &b_, &a_) == 0);
    requireSDLCondition(SDL_GetRenderDrawBlendMode(renderer_.get(), &blend_) == 0);
    SDL_RenderGetViewport(renderer_.get(), &savedViewport_);
    SDL_RenderGetClipRect(renderer_.get(), &savedClip_);
};

~RendererGuard() {
    requireSDLCondition(SDL_SetRenderTarget(renderer_.get(), savedTarget_.get()) == 0);

    requireSDLCondition(SDL_SetRenderDrawColor(renderer_.get(), r_, g_, b_, a_) == 0);
    requireSDLCondition(SDL_SetRenderDrawBlendMode(renderer_.get(), blend_) == 0);

    if (!isNullRect(savedViewport_))
        requireSDLCondition(SDL_RenderSetViewport(renderer_.get(), &savedViewport_) == 0);
    else
        requireSDLCondition(SDL_RenderSetViewport(renderer_.get(), nullptr) == 0);

    if (!isNullRect(savedClip_))
        requireSDLCondition(SDL_RenderSetClipRect(renderer_.get(), &savedClip_) == 0);
    else
        requireSDLCondition(SDL_RenderSetClipRect(renderer_.get(), nullptr) == 0);
    }
};

}