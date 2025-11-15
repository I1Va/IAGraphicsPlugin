#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <typeinfo>
#include <cassert>
#include <iostream>

#include "dr4/window.hpp"

#include "Drawable.hpp"


namespace ia {



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

    ~Window() override {
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
    }

    void SetTitle(const std::string &title) override { title_ = title; }
    const std::string &GetTitle() const override { return title_; }
    
    dr4::Vec2f GetSize() const override { return size_; };
    void SetSize(const ::dr4::Vec2f& size) { 
        size_ = size; 
        SDL_SetWindowSize(window_, size_.x, size_.y);
    }

    void Open() override {
        SDL_ShowWindow(window_);
        isOpen_ = true;
    };
    bool IsOpen() const override { return isOpen_; }
    void Close() override {
        SDL_HideWindow(window_);
        isOpen_ = false;
    }

    void Clear(dr4::Color color) override {
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_Rect full = {0, 0, static_cast<int>(size_.x), static_cast<int>(size_.y)};
        SDL_RenderFillRect(renderer_, &full);
    };

    void Draw(const dr4::Texture &texture) override {
        try {
            const Texture &src = dynamic_cast<const Texture &>(texture);

            SDL_Rect dstRect = SDL_Rect(pos.x, pos.y, src.size_.x, src.size_.y);
            SDL_RenderCopy(renderer_, src.texture_, NULL, &dstRect);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast: " << e.what() << '\n';
        }
    }

    void Display() override { SDL_RenderPresent(renderer_); }


    dr4::Texture *CreateTexture() { return new Texture(renderer_, 100, 100); }
    dr4::Image   *CreateImage() override { return new Image(); }
    dr4::Font    *CreateFont() override { return new Font(); }

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
                dr4Event.key.sym = convertToDr4KeyCode(SDLEvent.key.keysym.sym);
                dr4Event.key.mod = convertToDr4KeyMode(SDLEvent.key.keysym.mod);
                return dr4Event;

            case SDL_KEYUP:
                dr4Event.type = dr4::Event::Type::KEY_UP;
                dr4Event.key.sym = convertToDr4KeyCode(SDLEvent.key.keysym.sym);
                dr4Event.key.mod = convertToDr4KeyMode(SDLEvent.key.keysym.mod);
                return dr4Event;

            case SDL_MOUSEWHEEL:
            {
                int mouseX = 0, mouseY = 0;
                SDL_GetMouseState(&mouseX, &mouseY);

                dr4Event.type = dr4::Event::Type::MOUSE_WHEEL;
                dr4Event.mouseWheel.pos = dr4::Vec2f(mouseX, mouseY);
                dr4Event.mouseWheel.delta = SDLEvent.wheel.y;
                return dr4Event;
            }

            case SDL_MOUSEBUTTONDOWN:
                dr4Event.type = dr4::Event::Type::MOUSE_DOWN;
                dr4Event.mouseButton.button = convertToDr4MouseButton(SDLEvent.button.button);
                dr4Event.mouseButton.pos = dr4::Vec2f(SDLEvent.button.x, SDLEvent.button.y);
                return dr4Event;

            case SDL_MOUSEBUTTONUP:
                dr4Event.type = dr4::Event::Type::MOUSE_UP;
                dr4Event.mouseButton.button = convertToDr4MouseButton(SDLEvent.button.button);
                dr4Event.mouseButton.pos = dr4::Vec2f(SDLEvent.button.x, SDLEvent.button.y);
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
};

}