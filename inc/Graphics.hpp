#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <typeinfo>
#include <cassert>
#include <iostream>

#include "dr4/window.hpp"
#include "misc/dr4_ifc.hpp"

namespace ia {

class Window;
class Texture;

SDL_Color convertToSDLColor(const dr4::Color &color);
dr4::KeyMode convertToDr4KeyMode(const Uint16 SDLKeyMod);
dr4::KeyCode convertToDr4KeyCode(const SDL_Keycode SDLKeySym);
dr4::MouseCode convertToDr4MouseButton(const Uint8 SDLButton);

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

struct Font : public dr4::Font {
    constexpr static int FONT_SIZE = 24;
    int size_ = FONT_SIZE;
    TTF_Font *font_;

    Font() = default;
    explicit Font(int size): size_(size) {}

    ~Font() override {
        if (font_) TTF_CloseFont(font_);
    }

    void loadFromFile( const std::string& path ) override {
        std::cout << "font load from file\n";
        if (font_) TTF_CloseFont(font_);
        font_ = TTF_OpenFont(path.c_str(), size_);
        if (!font_) {
            printf("TTF_open font failed: %s\n", TTF_GetError());
        }
    }

    void setFontSize(const int newFontSize) const {
        assert(font_);
    
        if (TTF_SetFontSize(font_, newFontSize) == -1) {
            printf("Resize failed: %s\n", TTF_GetError());
        }
    }
};


class Image : public dr4::Image {
    static constexpr int BIT_PER_PIXEL = 32;
    static constexpr int R_MASK = 0x00FF0000;
    static constexpr int G_MASK = 0x0000FF00;
    static constexpr int B_MASK = 0x000000FF;
    static constexpr int A_MASK = 0xFF000000;

    SDL_Surface* surface_;

public:
    Image(const int width=100, const int height=100) {
        surface_ = createSDLSurface(width, height);
    }

    ~Image() override { if (surface_) SDL_FreeSurface(surface_); }

    void SetPixel(unsigned x, unsigned y, dr4::Color color) override {
        assert(surface_->format->BytesPerPixel == 4);
        assert(sizeof(dr4::Color) == 4);

        Uint8* pixel_ptr = (Uint8*)surface_->pixels
                            + y * surface_->pitch
                            + x * (surface_->format->BytesPerPixel);

        *(dr4::Color*)pixel_ptr = color;
    }

    dr4::Color GetPixel(unsigned x, unsigned y) const override {
        assert(surface_->format->BytesPerPixel == 4);
        assert(sizeof(dr4::Color) == 4);

        Uint8* pixel_ptr = (Uint8*)surface_->pixels
                            + y * surface_->pitch
                            + x * (surface_->format->BytesPerPixel);

        return *(dr4::Color*)pixel_ptr;
    }

    void SetSize(dr4::Vec2f size) override {
        assert(surface_);
        SDL_Surface* newSurface = createSDLSurface(size.x, size.y);
        if (newSurface) {
            SDL_FreeSurface(surface_);
            surface_ = newSurface;
        } else {
            SDL_Log("Failed to resize surface");
        }
    }

    dr4::Vec2f GetSize() const override {
        return dr4::Vec2f(surface_->w, surface_->h);
    }

    float GetWidth() const override {
        return surface_->w;
    }
    float GetHeight() const override {
        return surface_->h;
    }

private:
    SDL_Surface *createSDLSurface(const int width, const int height) {
        SDL_Surface *result = SDL_CreateRGBSurface(
            0,              // flags (0 for no special flags)
            width, height,  // width, height
            BIT_PER_PIXEL,  // bits per pixel
            R_MASK,         // R mask
            G_MASK,         // G mask
            B_MASK,         // B mask
            A_MASK          // A mask
        );


        if (!result) {
            SDL_Log("Failed to create surface: %s", SDL_GetError());
        }

        return result;
    }

    friend class Texture;
    friend class WIndow;
};

class Texture : public dr4::Texture {
    SDL_Renderer *renderer_;
    dr4::Vec2f size_;
    SDL_Texture* texture_;

public:
    ~Texture() override { if (texture_) SDL_DestroyTexture(texture_); };

    Texture(SDL_Renderer *renderer, int width=0, int height=0):
        renderer_(renderer), size_(width, height), texture_(nullptr) { 
        assert(renderer);
        if (width > 0 && height > 0) {
            texture_ = SDL_CreateTexture(renderer_, 
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    width, height);
            if (!texture_) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());
            }
    }
}

    void SetSize(dr4::Vec2f size) override { 
        if (texture_) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
        }
        size_ = size;
        if (size_.x > 0 && size_.y > 0) {
            texture_ = SDL_CreateTexture(renderer_, 
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    size_.x, size_.y);
        }
    }

    dr4::Vec2f GetSize() const override { return size_; }
    float GetWidth() const override { return size_.x; }
    float GetHeight() const override { return size_.y; }

    void Clear(dr4::Color color) override {
        RendererGuard renderGuard(renderer_);
        SDL_SetRenderTarget(renderer_, texture_);
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_Rect full = SDL_Rect{0, 0, static_cast<int>(size_.x), static_cast<int>(size_.y)};
        SDL_RenderFillRect(renderer_, &full);
    }

    void Draw(const dr4::Rectangle &rectangle) override {
        try {
            const dr4::Rectangle &srcRectangle = rectangle;

            RendererGuard renderGuard(renderer_);

            SDL_SetRenderTarget(renderer_, texture_);

            SDL_Rect borderRect = SDL_Rect(srcRectangle.rect.pos.x, srcRectangle.rect.pos.y, srcRectangle.rect.size.x, srcRectangle.rect.size.y);
            SDL_SetRenderDrawColor(renderer_, srcRectangle.borderColor.r, srcRectangle.borderColor.g, srcRectangle.borderColor.b, srcRectangle.borderColor.a);
            SDL_RenderFillRect(renderer_, &borderRect);

            if (srcRectangle.borderThickness * 2 < std::min(srcRectangle.rect.size.x, srcRectangle.rect.size.y)) {
                SDL_Rect innerRect = SDL_Rect
                (
                    srcRectangle.rect.pos.x  + srcRectangle.borderThickness,
                    srcRectangle.rect.pos.y  + srcRectangle.borderThickness,
                    srcRectangle.rect.size.x - 2 * srcRectangle.borderThickness,
                    srcRectangle.rect.size.y - 2 * srcRectangle.borderThickness
                );

                SDL_SetRenderDrawColor(renderer_, srcRectangle.fill.r, srcRectangle.fill.g, srcRectangle.fill.b, srcRectangle.fill.a);
                SDL_RenderFillRect(renderer_, &innerRect);
            }

        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast: " << e.what() << '\n';
        }
    }

    void Draw(const dr4::Text &text) override {
        try {
            const Font *font = dynamic_cast<const Font *>(text.font);
            if (!font) return;

            font->setFontSize(text.fontSize);

            SDL_Color textColor = convertToSDLColor(text.color);
            SDL_Surface *text_surface = TTF_RenderText_Blended(font->font_, text.text.c_str(), textColor);
            
            if (!text_surface) {
                std::cerr << "Failed to create text_surface: " << SDL_GetError() << '\n';
                return;
            }

            SDL_Texture* tempTexture = SDL_CreateTextureFromSurface(renderer_, text_surface);
            if (!tempTexture) {
                std::cerr << "Failed to create texture from surface: " << SDL_GetError() << '\n';
                return;
            }
         
            RendererGuard renderGuard(renderer_);

            SDL_SetRenderTarget(renderer_, texture_);

            dr4::Rect2f textBounds = text.GetBounds();
            SDL_Rect dstRect = {
                static_cast<int>(textBounds.pos.x),
                static_cast<int>(textBounds.pos.y),
                static_cast<int>(textBounds.size.x),
                static_cast<int>(textBounds.size.y)
            };
    
            SDL_RenderCopy(renderer_, tempTexture, nullptr, &dstRect);

            SDL_DestroyTexture(tempTexture);
            SDL_FreeSurface(text_surface);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Text: " << e.what() << '\n';
        }
    }

    void Draw(const dr4::Image &img, const dr4::Vec2f &pos) override {
        try {
              const Image &srcImage = dynamic_cast<const Image &>(img);

            SDL_Texture* tempTexture = SDL_CreateTextureFromSurface(renderer_, srcImage.surface_);
            if (!tempTexture) {
                std::cerr << "Failed to create texture from surface: " << SDL_GetError() << '\n';
                return;
            }
            
            RendererGuard renderGuard(renderer_);

            SDL_SetRenderTarget(renderer_, texture_);

            SDL_Rect dstRect = {
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                static_cast<int>(srcImage.GetWidth()),
                static_cast<int>(srcImage.GetHeight())
            };
    
            SDL_RenderCopy(renderer_, tempTexture, nullptr, &dstRect);

            SDL_DestroyTexture(tempTexture);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Image: " << e.what() << '\n';
        }
    }

    void Draw(const dr4::Texture &texture, const dr4::Vec2f &pos) override {
        try {
            const Texture &srcTexture = dynamic_cast<const Texture &>(texture);
            
            RendererGuard renderGuard(renderer_);
            SDL_SetRenderTarget(renderer_, texture_);

            SDL_Rect dstRect = {
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                static_cast<int>(srcTexture.GetWidth()),
                static_cast<int>(srcTexture.GetHeight())
            };

            SDL_RenderCopy(renderer_, srcTexture.texture_, nullptr, &dstRect);

        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Texture: " << e.what() << '\n';
        }
    }

    friend class Window;
};

class Window : public dr4::Window {
    SDL_Renderer *renderer_ = nullptr;
    SDL_Window *window_ = nullptr;
    std::string title_;
    dr4::Vec2f size_;
    bool isOpen_ = false;

public:
    ~Window() override {
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
    }

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
