#pragma once
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "dr4/texture.hpp"
#include "dr4/math/rect.hpp"
#include "Common.hpp"

namespace ia {

class Texture : public dr4::Texture {
    SDL_Renderer *renderer_;
    dr4::Vec2f size_;
    SDL_Texture* texture_;

    dr4::Vec2f pos_;
    dr4::Vec2f zero_;

    friend Line;
    friend Circle;
    friend Rectangle;

public:
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

            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND); 
            SDL_SetTextureAlphaMod(texture_, 255);
        }
    }

    ~Texture() override { if (texture_) SDL_DestroyTexture(texture_); };

    void DrawOn(dr4::Texture& texture) const override {
        try {
            const Texture &srcTexture = dynamic_cast<const Texture &>(texture);
            
            RendererGuard renderGuard(renderer_);
            SDL_SetRenderTarget(renderer_, texture_);

            SDL_Rect dstRect = {
                static_cast<int>(pos_.x),
                static_cast<int>(pos_.y),
                static_cast<int>(srcTexture.GetWidth()),
                static_cast<int>(srcTexture.GetHeight())
            };

            SDL_RenderCopy(renderer_, srcTexture.texture_, nullptr, &dstRect);

        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Texture: " << e.what() << '\n';
        }
    }
    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; }

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

    void SetZero(dr4::Vec2f pos) override { zero_ = pos; }
    dr4::Vec2f GetZero() const override { return zero_; }

    void Clear(dr4::Color color) override {
        RendererGuard renderGuard(renderer_);
        SDL_SetRenderTarget(renderer_, texture_);
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_Rect full = SDL_Rect{0, 0, static_cast<int>(size_.x), static_cast<int>(size_.y)};
        SDL_RenderFillRect(renderer_, &full);
    }
};

class Line : public dr4::Line {
    dr4::Vec2f start_;
    dr4::Vec2f end_;
    float thickness_;
    SDL_Color color_;

public:
    Line() = default;
    Line(dr4::Vec2f start, dr4::Vec2f end, float thickness, SDL_Color color):
        start_(start), end_(end), thickness_(thickness), color_(color) {}
    ~Line() override = default;

    void DrawOn(dr4::Texture &texture) const override {
        try {
            const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
            
            RendererGuard renderGuard(dstTexture.renderer_);
            SDL_SetRenderTarget(dstTexture.renderer_, dstTexture.texture_);

            thickLineColor(dstTexture.renderer_, 
                           static_cast<Sint16>(start_.x + dstTexture.zero_.x), 
                           static_cast<Sint16>(start_.y + dstTexture.zero_.y), 
                           static_cast<Sint16>(end_.x + dstTexture.zero_.x), 
                           static_cast<Sint16>(end_.y + dstTexture.zero_.y), 
                           static_cast<Uint8>(thickness_), SDLColorToGfxColor(color_));
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Texture: " << e.what() << '\n';
        }
    }

    void SetStart(dr4::Vec2f start) override { start_ = start; }
    void SetEnd(dr4::Vec2f end) override { end_ = end; }
    void SetColor(dr4::Color color) override { color_ = convertToSDLColor(color); }
    void SetThickness(float thickness) override { thickness_ = thickness; }

    dr4::Vec2f GetStart() const override { return start_; }
    dr4::Vec2f GetEnd() const override { return end_; }
    dr4::Color GetColor() const override { return dr4::Color(color_.r, color_.g, color_.b, color_.a); }
    float GetThickness() const override { return thickness_; }
};

class Circle : public dr4::Circle {
    dr4::Vec2f pos_;
    float radius_;
    float borderThickness_;

    SDL_Color fillColor_;
    SDL_Color borderColor_;
    
public:
    Circle() = default;
    Circle(dr4::Vec2f pos, float radius, float borderThickness, 
           SDL_Color fillColor, SDL_Color borderColor):
        pos_(pos), radius_(radius), borderThickness_(borderThickness), 
        fillColor_(fillColor), borderColor_(borderColor) {}
    ~Circle() override = default;

    void DrawOn(dr4::Texture &texture) const override {
        try {
            const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
            
            RendererGuard renderGuard(dstTexture.renderer_);
            SDL_SetRenderTarget(dstTexture.renderer_, dstTexture.texture_);

            std::cerr << "Bordered circles are not supported\n";

            filledCircleColor(dstTexture.renderer_, 
                static_cast<Sint16>(pos_.x), static_cast<Sint16>(pos_.y), 
                static_cast<Sint16>(radius_), SDLColorToGfxColor(fillColor_));         
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Texture: " << e.what() << '\n';
        }
    }

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }

    dr4::Vec2f GetPos() const override { return pos_; }

    void SetCenter(dr4::Vec2f center) override { pos_ = center; }
    void SetRadius(float radius) override { radius_ = radius; }
    void SetFillColor(dr4::Color color) override { fillColor_ = convertToSDLColor(color); }
    void SetBorderColor(dr4::Color color) override {borderColor_ = convertToSDLColor(color); }
    void SetBorderThickness(float thickness) override { borderThickness_ = thickness; }

    dr4::Vec2f GetCenter() const override { return pos_; }
    float GetRadius() const override { return radius_; }
    dr4::Color GetFillColor() const override { return convertToDr4Color(fillColor_); }
    dr4::Color GetBorderColor() const override { return convertToDr4Color(borderColor_); };
    float GetBorderThickness() const override { return borderThickness_; }
};

class Rectangle : public dr4::Rectangle {
    dr4::Rect2f rect_;

    float borderThickness_;
    SDL_Color fillColor_;
    SDL_Color borderColor_;

public:
    Rectangle() = default;
    Rectangle(dr4::Vec2f pos, dr4::Vec2f size, float borderThickness,
              SDL_Color fillColor, SDL_Color borderColor):
        rect_(pos, size), borderThickness_(borderThickness), 
        fillColor_(fillColor), borderColor_(borderColor) {}
    ~Rectangle() override = default;

    void DrawOn(dr4::Texture& texture) const override {
        try {
            const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
            
            RendererGuard renderGuard(dstTexture.renderer_);
            SDL_SetRenderTarget(dstTexture.renderer_, dstTexture.texture_);

            if (2 * borderThickness_ >= std::fmin(rect_.size.x, rect_.size.y)) {
                SDL_SetRenderDrawColor(dstTexture.renderer_, borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a);
                SDL_Rect innerRect = convertToSDLRect(rect_);
                SDL_RenderFillRect(dstTexture.renderer_, &innerRect);
                return;
            }

            SDL_Rect innerRect = SDL_Rect
            (
                static_cast<int>(rect_.pos.x + borderThickness_),
                static_cast<int>(rect_.pos.y + borderThickness_),
                static_cast<int>(rect_.size.x - 2 * borderThickness_),
                static_cast<int>(rect_.size.y - 2 * borderThickness_)
            );

            SDL_SetRenderDrawColor(dstTexture.renderer_, fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a);
            SDL_RenderFillRect(dstTexture.renderer_, &innerRect);
    
            SDL_SetRenderDrawColor(dstTexture.renderer_, borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a);

            SDL_Rect top = SDL_Rect
            (
                static_cast<int>(rect_.pos.x),
                static_cast<int>(rect_.pos.y),
                static_cast<int>(rect_.size.x),
                static_cast<int>(borderThickness_)
            );
            SDL_RenderFillRect(dstTexture.renderer_, &top);


            SDL_Rect bottom = SDL_Rect
            (
                static_cast<int>(rect_.pos.x),
                static_cast<int>(rect_.pos.y + rect_.size.y - borderThickness_),
                static_cast<int>(rect_.size.x),
                static_cast<int>(borderThickness_)
            );
            SDL_RenderFillRect(dstTexture.renderer_, &bottom);

            SDL_Rect left = SDL_Rect
            (
                static_cast<int>(rect_.pos.x),
                static_cast<int>(rect_.pos.y),
                static_cast<int>(borderThickness_),
                static_cast<int>(rect_.size.y)
            );
            SDL_RenderFillRect(dstTexture.renderer_, &left);

            SDL_Rect right = SDL_Rect
            (
                static_cast<int>(rect_.pos.x + rect_.size.y - borderThickness_),
                static_cast<int>(rect_.pos.y),
                static_cast<int>(borderThickness_),
                static_cast<int>(rect_.size.y)
            );
            SDL_RenderFillRect(dstTexture.renderer_, &right);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Texture: " << e.what() << '\n';
        }
    }

    void SetPos(dr4::Vec2f pos) override { rect_.pos = pos; }
    dr4::Vec2f GetPos() const override { return rect_.pos; }

    void SetSize(dr4::Vec2f size) override { rect_.size = size; }
    void SetFillColor(dr4::Color color) override { fillColor_ = convertToSDLColor(color); }
    void SetBorderThickness(float thickness) override { borderThickness_ = thickness; }
    void SetBorderColor(dr4::Color color) override { borderColor_ = convertToSDLColor(color);}

    dr4::Vec2f GetSize() const override { return rect_.size; }
    dr4::Color GetFillColor() const override { return convertToDr4Color(fillColor_); }
    float GetBorderThickness() const override { return borderThickness_; }
    dr4::Color GetBorderColor() const override { return convertToDr4Color(borderColor_); }
};

class Font : public dr4::Font {
    constexpr static int DEFAULT_FONT_SIZE = 24;
    TTF_Font *font_ = nullptr;
    int fontSize_ = DEFAULT_FONT_SIZE;

public:
    Font() = default;
    ~Font() override { TTF_CloseFont(font_); };

    void LoadFromFile(const std::string& path) override {
        if (font_) TTF_CloseFont(font_);
        font_ = TTF_OpenFont(path.c_str(), fontSize_);
        if (!font_) {
            printf("TTF_open font failed: %s\n", TTF_GetError());
        }
    }

    void LoadFromBuffer(const void *buffer, size_t size) override {
        SDL_RWops *rw = SDL_RWFromConstMem(buffer, size);
        font_ = TTF_OpenFontRW(rw, 1, fontSize_); // frees SDL_RWops rw automatically
    }

    float GetAscent(float fontSize) const override { return TTF_FontAscent(font_); }
    float GetDescent(float fontSize) const override { return TTF_FontDescent(font_); }
};


}
