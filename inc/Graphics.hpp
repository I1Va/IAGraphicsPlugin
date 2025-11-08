#pragma once
#include <iostream>
#include <cassert>
#include <SDL2/SDL.h>

#include "dr4/window.hpp"
#include "misc/dr4_ifc.hpp"

namespace ia {

class Window;
class Texture;

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
    savedTarget_ = SDL_GetRenderTarget(renderer_);
    SDL_GetRenderDrawColor(renderer_, &r_, &g_, &b_, &a_);
    SDL_GetRenderDrawBlendMode(renderer_, &blend_);
    SDL_RenderGetViewport(renderer_, &savedViewport_);
    SDL_RenderGetClipRect(renderer_, &savedClip_);
};

~RendererGuard() {
    if (renderer_)
        SDL_SetRenderTarget(renderer_, savedTarget_); // restore previous render target
            
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

class Texture : public dr4::Texture {
    SDL_Renderer *renderer_;
    dr4::Vec2f size_;
    SDL_Texture* texture_;

public: 
    Texture(SDL_Renderer *renderer, int width=0, int height=0): renderer_(renderer), size_(width, height) {}

    void SetSize(dr4::Vec2f size) override {
        size_ = size; 
        texture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            size_.x, size_.y
        );
    }

    dr4::Vec2f GetSize() const override { return size_; }
    float GetWidth() const override { return size_.x; }
    float GetHeight() const override { return size_.y; }
    
    void Draw(const dr4::Rectangle &rect) override {
        RendererGuard renderGuard(renderer_);

        SDL_SetRenderTarget(renderer_, texture_);

        SDL_Rect borderRect = SDL_Rect(rect.rect.pos.x, rect.rect.pos.y, rect.rect.size.x, rect.rect.size.y);
        SDL_SetRenderDrawColor(renderer_, rect.borderColor.r, rect.borderColor.g, rect.borderColor.b, rect.borderColor.a);
        SDL_RenderFillRect(renderer_, &borderRect);

        if (rect.borderThickness * 2 < std::min(rect.rect.size.x, rect.rect.size.y)) {
            SDL_Rect innerRect = 
            SDL_Rect(
                rect.rect.pos.x  + rect.borderThickness,
                rect.rect.pos.y  + rect.borderThickness, 
                rect.rect.size.x - 2 * rect.borderThickness, 
                rect.rect.size.y - 2 * rect.borderThickness
            );
            std::cout << innerRect.x << " " << innerRect.y << " " << innerRect.w << " " << innerRect.h << "\n";
            SDL_SetRenderDrawColor(renderer_, rect.fill.r, rect.fill.g, rect.fill.b, rect.fill.a);
            SDL_RenderFillRect(renderer_, &innerRect);
        }
    }

    void Draw(const dr4::Text &text) override {}
    void Draw(const dr4::Texture &texture, const dr4::Vec2f &pos) override {
        RendererGuard renderGuard(renderer_);
    
        try {
            const Texture &src = dynamic_cast<const Texture &>(texture);

            SDL_SetRenderTarget(renderer_, texture_);
            SDL_Rect dstRect = SDL_Rect(pos.x, pos.y, src.size_.x, src.size_.y);
            SDL_RenderCopy(renderer_, src.texture_, NULL, &dstRect);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast: " << e.what() << '\n';
        }
    }

    inline ~Texture() {};

    friend class Window;
};

class Window : public dr4::Window {
    SDL_Renderer *renderer_ = nullptr;
    SDL_Window *window_ = nullptr;
    std::string title_;
    dr4::Vec2f size_;
    bool isOpen_ = false;

public:
    Window
    (
        const std::string &title,
        const int width=100,
        const int height=100
    ) : title_(title), size_(width, height)
    {
        window_ = SDL_CreateWindow(
            title_.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            size_.x, size_.y,
            SDL_WINDOW_HIDDEN
        );

        if (!window_) {
            printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
            SDL_Quit();
            // return 1;
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer_) {
            printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window_);
            SDL_Quit();
            // return 1;
        }
    }

    void SetTitle(const std::string &title) override { title_ = title; }
    const std::string &GetTitle() const override { return title_; }
    dr4::Vec2f GetSize() const override { return size_; };
    void SetSize(const ::dr4::Vec2f& size) override { size_ = size; }

    void Open() override { 
        SDL_ShowWindow(window_);
        isOpen_ = true;
    }

    bool IsOpen() const override {
        return isOpen_;
    }

    void Close() override { 
        SDL_HideWindow(window_); 
        isOpen_ = false;
    }

    void Clear(const dr4::Color &color) override {
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_Rect full = SDL_Rect(0, 0, size_.x, size_.y);
        SDL_RenderFillRect(renderer_, &full);
    };

    void Draw(const dr4::Texture &texture, dr4::Vec2f pos) override {
        try {
            const Texture &src = dynamic_cast<const Texture &>(texture);
    
            SDL_Rect dstRect = SDL_Rect(pos.x, pos.y, src.size_.x, src.size_.y);
            SDL_RenderCopy(renderer_, src.texture_, NULL, &dstRect);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast: " << e.what() << '\n';
        }
    }

    void Display() override { 
        std::cout << "Display\n";
        SDL_RenderPresent(renderer_); 
    }

    dr4::Texture *CreateTexture() override { return new Texture(renderer_); }
    
    std::optional<dr4::Event> PollEvent() override {
        SDL_Event SDLEvent{};
        dr4::Event dr4Event{};
        static int prevMouseX = 0, prevMouseY = 0;

        if (!SDL_PollEvent(&SDLEvent)) return std::nullopt;
        switch (SDLEvent.type) {
            case SDL_QUIT:
                dr4Event.type = dr4::Event::Type::QUIT;
                return dr4Event;
            
            case SDL_KEYDOWN:
                dr4Event.type = dr4::Event::Type::KEY_DOWN;
                dr4Event.keyDown.sym = static_cast<dr4::KeySyms>(SDLEvent.key.keysym.sym);
                dr4Event.keyDown.mod = static_cast<dr4::KeyModes>(SDLEvent.key.keysym.mod);
                return dr4Event;
            
            case SDL_KEYUP:
                dr4Event.type = dr4::Event::Type::KEY_UP;
                dr4Event.keyDown.sym   = static_cast<dr4::KeySyms>(SDLEvent.key.keysym.sym);
                dr4Event.keyDown.mod   = static_cast<dr4::KeyModes>(SDLEvent.key.keysym.mod);
                return dr4Event;
                
            case SDL_MOUSEWHEEL:
            {
                int mouseX = 0, mouseY = 0;
                SDL_GetMouseState(&mouseX, &mouseY);

                dr4Event.type = dr4::Event::Type::MOUSE_WHEEL;
                dr4Event.mouseWheel.pos = dr4::Vec2f(mouseX, mouseY);
                dr4Event.mouseWheel.delta = dr4::Vec2f(SDLEvent.wheel.x, SDLEvent.wheel.y);
                return dr4Event;
            }
        
            case SDL_MOUSEBUTTONDOWN:
                dr4Event.type = dr4::Event::Type::MOUSE_DOWN;
                dr4Event.mouseButton.button = SDLEvent.button.button;
                dr4Event.mouseButton.pos = dr4::Vec2f(SDLEvent.button.x, SDLEvent.button.y);
                dr4Event.mouseButton.pressed = true;
                return dr4Event;

            case SDL_MOUSEBUTTONUP:
                dr4Event.type = dr4::Event::Type::MOUSE_UP;
                dr4Event.mouseButton.button = SDLEvent.button.button;
                dr4Event.mouseButton.pos = dr4::Vec2f(SDLEvent.button.x, SDLEvent.button.y);
                dr4Event.mouseButton.pressed = false;
                return dr4Event;
            
            case SDL_MOUSEMOTION:
            {
                int mouseX = 0, mouseY = 0;
                SDL_GetMouseState(&mouseX, &mouseY);
                
                dr4Event.type = dr4::Event::Type::MOUSE_MOVE;
                dr4Event.mouseMove.pos = dr4::Vec2f(mouseX, mouseY);
                dr4Event.mouseMove.rel = dr4::Vec2f(mouseX - prevMouseX, mouseY - prevMouseY);

                return dr4Event;
            }    

            default:
                break;
        }
        SDL_GetMouseState(&prevMouseX, &prevMouseY);
        return std::nullopt;
    }

    ~Window() {
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        SDL_Quit();
    }
    
};

class IAGraphicsBackEnd : public dr4::DR4Backend {
    const std::string name_ = "IAGraphicsBackEnd";

public:
    ~IAGraphicsBackEnd() { SDL_Quit(); }

    IAGraphicsBackEnd() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("IAGraphicsBackEnd : SDL_Init Error. %s\n", SDL_GetError());
            // return 1;
        }
    }

    const std::string &Name() const override { return name_; }

    dr4::Window *CreateWindow() override { return new ia::Window("Window"); }
};

}