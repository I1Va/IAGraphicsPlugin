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

static constexpr size_t BUFER_SIZE = 32;
static char BUFER[BUFER_SIZE] = {};

class Window : public dr4::Window {
    raii::SDL_Renderer renderer_;
    raii::SDL_Window window_;
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
        window_ = raii::SDL_CreateWindow(
            title_.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            size_.x, size_.y,
            SDL_WINDOW_HIDDEN
        );
        requireSDLCondition(window_ != nullptr);

        renderer_ = raii::SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        requireSDLCondition(renderer_ != nullptr);

        requireSDLCondition(SDL_SetRenderDrawBlendMode(renderer_.get(), SDL_BLENDMODE_BLEND) == 0);
    }

    ~Window() = default;

    void SetTitle(const std::string &title) override { title_ = title; }
    const std::string &GetTitle() const override { return title_; }
    
    dr4::Vec2f GetSize() const override { return size_; };
    void SetSize(dr4::Vec2f size) override { 
        size_ = size; 
        SDL_SetWindowSize(window_.get(), size_.x, size_.y);
    }

    void Open() override {
        SDL_ShowWindow(window_.get());
        isOpen_ = true;
    };
    bool IsOpen() const override { return isOpen_; }
    void Close() override {
        SDL_HideWindow(window_.get());
        isOpen_ = false;
    }

    void Clear(dr4::Color color) override {
        RendererGuard renderGuard(renderer_);
        SDL_SetRenderDrawColor(renderer_.get(), color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer_.get());
    };

    void Draw(const dr4::Texture &texture) override try{        
        const Texture &src = dynamic_cast<const Texture &>(texture);
        RendererGuard rendererGuard(renderer_);
        
        SDL_Rect dstRect = SDL_Rect(src.GetPos().x, src.GetPos().y, src.GetWidth(), src.GetHeight());
        
        SDL_RenderCopy(renderer_.get(), src.texture_.get(), nullptr, &dstRect);
    } catch (const std::bad_cast& e) { std::throw_with_nested(Dr4Exception("dynamic_cast failed in Texture::drawOn")); }

    void Display() override { SDL_RenderPresent(renderer_.get()); }

    double GetTime() override { return static_cast<double>(SDL_GetTicks64()) / 1000; }
    void Sleep(double time) override { SDL_Delay(static_cast<Uint32> (time * 1000));}
    Texture   *CreateTexture()   override { return new Texture(*this); }
    Image     *CreateImage()     override { return new Image(); }
    Font      *CreateFont()      override { return new Font(); }
    Line      *CreateLine()      override { return new Line(); }
    Circle    *CreateCircle()    override { return new Circle(); }
    Rectangle *CreateRectangle() override { return new Rectangle(); }
    Text      *CreateText()      override { return new Text(); }

    void StartTextInput() override { SDL_StartTextInput(); }
    void StopTextInput() override { SDL_StopTextInput(); }

    std::optional<dr4::Event> PollEvent() override {
        SDL_Event SDLEvent{};
        std::optional<dr4::Event> dr4Event = dr4::Event();
        dr4Event.value().text.unicode = BUFER;

        static int prevMouseX = 0, prevMouseY = 0;

        if (!SDL_PollEvent(&SDLEvent)) return std::nullopt;
        switch (SDLEvent.type) {
            case SDL_QUIT:
                dr4Event.value().type = dr4::Event::Type::QUIT;
                break;

            case SDL_KEYDOWN:
                dr4Event.value().type = dr4::Event::Type::KEY_DOWN;
                dr4Event.value().key.sym = convertToDr4KeyCode(SDLEvent.key.keysym.sym);
                dr4Event.value().key.mods = convertToDr4KeyMode(SDLEvent.key.keysym.mod);
                break;

            case SDL_KEYUP:
                dr4Event.value().type = dr4::Event::Type::KEY_UP;
                dr4Event.value().key.sym = convertToDr4KeyCode(SDLEvent.key.keysym.sym);
                dr4Event.value().key.mods = convertToDr4KeyMode(SDLEvent.key.keysym.mod);
                break;

            case SDL_MOUSEWHEEL:
            {
                int mouseX = 0, mouseY = 0;
                SDL_GetMouseState(&mouseX, &mouseY);

                dr4Event.value().type = dr4::Event::Type::MOUSE_WHEEL;
                dr4Event.value().mouseWheel.pos = dr4::Vec2f(mouseX, mouseY);
                dr4Event.value().mouseWheel.delta = dr4::Vec2f(SDLEvent.wheel.x, SDLEvent.wheel.y);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                dr4Event.value().type = dr4::Event::Type::MOUSE_DOWN;
                dr4Event.value().mouseButton.button = convertToDr4MouseButton(SDLEvent.button.button);
                dr4Event.value().mouseButton.pos = dr4::Vec2f(SDLEvent.button.x, SDLEvent.button.y);
                break;

            case SDL_MOUSEBUTTONUP:
                dr4Event.value().type = dr4::Event::Type::MOUSE_UP;
                dr4Event.value().mouseButton.button = convertToDr4MouseButton(SDLEvent.button.button);
                dr4Event.value().mouseButton.pos = dr4::Vec2f(SDLEvent.button.x, SDLEvent.button.y);
                break;

            case SDL_MOUSEMOTION:
            {
                int mouseX = 0, mouseY = 0;
                SDL_GetMouseState(&mouseX, &mouseY);
                dr4Event.value().type = dr4::Event::Type::MOUSE_MOVE;
                dr4Event.value().mouseMove.pos = dr4::Vec2f(mouseX, mouseY);
                dr4Event.value().mouseMove.rel = dr4::Vec2f(mouseX - prevMouseX, mouseY - prevMouseY);
                break;
            }

            case SDL_TEXTINPUT:
            {
                dr4Event.value().type = dr4::Event::Type::TEXT_EVENT;
                strncpy(const_cast<char *>(dr4Event.value().text.unicode), SDLEvent.text.text, BUFER_SIZE);
                break;
            }
                 

            default:
                dr4Event = std::nullopt;
                break;
        }
        SDL_GetMouseState(&prevMouseX, &prevMouseY);
        return dr4Event;
    }

    const raii::SDL_Renderer &getRenderer() const { return renderer_; }
};

}