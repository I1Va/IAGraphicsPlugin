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

    friend class Line;
    friend class Circle;
    friend class Rectangle;
    friend class Text;
    friend class Image;

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
            const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
            assert(texture_ && dstTexture.texture_);
    
            RendererGuard renderGuard(renderer_);
            SDL_SetRenderTarget(dstTexture.renderer_, dstTexture.texture_);

            SDL_Rect dstRect = { dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y, dstTexture.GetWidth(), dstTexture.GetHeight() };

            SDL_RenderCopy(dstTexture.renderer_, dstTexture.texture_, nullptr, &dstRect);

        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Draw Texture::drawOn: " << e.what() << '\n';
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
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(texture_, 255);
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
        SDL_Rect full = SDL_Rect{0, 0, size_.x, size_.y};
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
                           dstTexture.zero_.x + start_.x, 
                           dstTexture.zero_.y + start_.y, 
                           dstTexture.zero_.x + end_.x, 
                           dstTexture.zero_.y + end_.y, 
                           thickness_, SDLColorToGfxColor(color_));
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Line::DrawOn: " << e.what() << '\n';
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
                dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y, radius_, SDLColorToGfxColor(fillColor_));         
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Circle::drawOn: " << e.what() << '\n';
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
                innerRect.x += dstTexture.zero_.x;
                innerRect.y += dstTexture.zero_.y;

                SDL_RenderFillRect(dstTexture.renderer_, &innerRect);
                return;
            }

            SDL_Rect innerRect = SDL_Rect
            (
                dstTexture.zero_.x + rect_.pos.x + borderThickness_,
                dstTexture.zero_.y + rect_.pos.y + borderThickness_,
                rect_.size.x - 2 * borderThickness_,
                rect_.size.y - 2 * borderThickness_
            );

            SDL_SetRenderDrawColor(dstTexture.renderer_, fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a);
            SDL_RenderFillRect(dstTexture.renderer_, &innerRect);
    
            SDL_SetRenderDrawColor(dstTexture.renderer_, borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a);

            SDL_Rect top = SDL_Rect
            (
                dstTexture.zero_.x + rect_.pos.x,
                dstTexture.zero_.y + rect_.pos.y,
                rect_.size.x,
                borderThickness_
            );
            SDL_RenderFillRect(dstTexture.renderer_, &top);


            SDL_Rect bottom = SDL_Rect
            (
                dstTexture.zero_.x + rect_.pos.x,
                dstTexture.zero_.y + rect_.pos.y + rect_.size.y - borderThickness_,
                rect_.size.x,
                borderThickness_
            );
            SDL_RenderFillRect(dstTexture.renderer_, &bottom);

            SDL_Rect left = SDL_Rect
            (
                dstTexture.zero_.x + rect_.pos.x,
                dstTexture.zero_.y + rect_.pos.y,
                borderThickness_,
                rect_.size.y
            );
            SDL_RenderFillRect(dstTexture.renderer_, &left);

            SDL_Rect right = SDL_Rect
            (
                dstTexture.zero_.x + rect_.pos.x + rect_.size.x - borderThickness_,
                dstTexture.zero_.y + rect_.pos.y,
                borderThickness_,
                rect_.size.y
            );
        
            SDL_RenderFillRect(dstTexture.renderer_, &right);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Rectangle::DrawOn: " << e.what() << '\n';
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

    std::optional<std::string> lastFileLoadpath{};
    SDL_RWops *lastLoadBufer = nullptr;

    friend Text;

public:
    Font() = default;
    ~Font() override { 
        if (font_) TTF_CloseFont(font_); 
        if (lastLoadBufer) SDL_RWclose(lastLoadBufer); 
    };

    void LoadFromFile(const std::string& path) override {
        resetFont();
        lastFileLoadpath = path; 

        font_ = TTF_OpenFont(lastFileLoadpath.value().c_str(), fontSize_);
        if (!font_) printf("TTF_open font failed: %s\n", TTF_GetError());
    }

    void LoadFromBuffer(const void *buffer, size_t size) override {
        assert(buffer);
    
        resetFont();
        lastLoadBufer = SDL_RWFromConstMem(buffer, size);
    
        font_ = TTF_OpenFontRW(lastLoadBufer, 0, fontSize_);
        if (!font_) {
            printf("TTF_open font failed: %s\n", TTF_GetError());
            resetFont();
        };
    }

    float GetAscent(float fontSize) const override { 
        if (!font_) {
            std::cerr << "font wasn't loaded\n";
            return 0.0;
        }

        return TTF_FontAscent(font_); 
    }

    float GetDescent(float fontSize) const override {
        if (!font_) {
            std::cerr << "font wasn't loaded\n";
            return 0.0;
        }

        return TTF_FontDescent(font_);
    }

    float getFontSize() const { return fontSize_; }
    void setFontSize(float fontSize) {
        assert(!(lastFileLoadpath.has_value() && (lastLoadBufer != nullptr)));

        if (static_cast<int> (fontSize) == fontSize_) return;
        fontSize_ = static_cast<int> (fontSize);

        if (lastFileLoadpath.has_value())
            LoadFromFile(lastFileLoadpath.value());
        else loadFromRWBUffer(lastLoadBufer);
    }

private:
    void resetFont() {
            if (font_) {
                TTF_CloseFont(font_);
                font_ = nullptr;
            }
            if (lastLoadBufer) {
                SDL_RWclose(lastLoadBufer);
                lastLoadBufer = nullptr;
            }
            lastFileLoadpath.reset();
        }

    void loadFromRWBUffer(SDL_RWops *bufer) {
        assert(bufer);
    
        if (font_) { TTF_CloseFont(font_); font_ = nullptr; }
        font_ = TTF_OpenFontRW(lastLoadBufer, 0, fontSize_);
        if (!font_) {
            printf("TTF_open font failed: %s\n", TTF_GetError());
            resetFont();
        };
    }
};

struct FontGuard {
    Font *font_;
    float savedFontSize_;

    FontGuard(Font *font): font_(font), savedFontSize_(font ? font->getFontSize() : 0) { assert(font); }
    ~FontGuard() { font_->setFontSize(savedFontSize_); }
};

class Text : public dr4::Text {
    constexpr static float DEFAULT_FONT_SIZE = 24;

    Font *font_ = nullptr;
    float fontSize_ = DEFAULT_FONT_SIZE;
    SDL_Color color_ = SDL_Color(0, 0, 0, 255);

    std::string text_ = "Text";
    dr4::Text::VAlign vAlign_ = dr4::Text::VAlign::TOP;
    dr4::Vec2f pos_{};

public:
    Text() = default;
    ~Text() override = default;

    void DrawOn(dr4::Texture& texture) const override {
        try {
            if (font_ == nullptr) {
                std::cerr << "font wasn't set\n";
                return; 
            }
            if (font_->font_ == nullptr) {
                std::cerr << "text font wasn't loaded\n";
                return;
            }

            const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
            RendererGuard renderGuard(dstTexture.renderer_);
            FontGuard fontGuard(font_);
            
            SDL_SetRenderTarget(dstTexture.renderer_, dstTexture.texture_);
            DrawTextDetail(dstTexture.renderer_, font_, text_.c_str(), 
                            dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y, vAlign_, color_);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Text::DrawOn: " << e.what() << '\n';
        }
    };

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; };

    void SetText(const std::string &text) override { text_ = text; }
    void SetColor(dr4::Color color) override { color_ = convertToSDLColor(color); }
    void SetFontSize(float size) override { fontSize_ = size; }
    void SetVAlign(dr4::Text::VAlign align) override { vAlign_ = align; }
    void SetFont(const dr4::Font *font) override { 
        auto f = dynamic_cast<const Font*>(font);
        if (!f) { std::cerr << "Bad cast in Text::SetFont\n"; return; }
        font_ = const_cast<Font*>(f);
    }

    dr4::Vec2f         GetBounds() const override { 
        FontGuard fontGuard(font_);

        font_->setFontSize(fontSize_);
        int textWidth, textHeight;
        TTF_SizeUTF8(font_->font_, text_.c_str(), &textWidth, &textHeight);

        return dr4::Vec2f(static_cast<float>(textWidth), static_cast<float>(textHeight));
    }

    const std::string &GetText() const override { return text_; }
    dr4::Color         GetColor() const override { return convertToDr4Color(color_); }
    float              GetFontSize() const override { return fontSize_; }
    VAlign             GetVAlign() const override { return vAlign_; }
    const Font        &GetFont() const override { return *font_; }

private:

    void DrawTextDetail(SDL_Renderer* renderer, Font* font, const char* text,
                        int x, int y, VAlign valign, SDL_Color color) const
    {
        font->setFontSize(fontSize_);
    
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font->font_, text, color);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);

        SDL_Rect dst = { x, y, w, h };

        switch (valign) {
            case VAlign::TOP:        break;
            case VAlign::MIDDLE:     dst.y -= h / 2; break;
            case VAlign::BASELINE:   dst.y -= TTF_FontAscent(font->font_); break;
            case VAlign::BOTTOM:     dst.y -= h; break;
            default: break;
        }

        SDL_RenderCopy(renderer, tex, nullptr, &dst);

        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }
};

class Image : public dr4::Image {
    static constexpr int BIT_PER_PIXEL = 32;

    dr4::Vec2f pos_{};
    SDL_Surface* surface_{};

public:
    Image() = delete;
    Image(const int width=100, const int height=100) {
        surface_ = createSDLSurface(width, height);
    }

    ~Image() override { if (surface_) SDL_FreeSurface(surface_); }

    void DrawOn(dr4::Texture &texture) const override {
        try {
            const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
            
            RendererGuard renderGuard(dstTexture.renderer_);
            SDL_SetRenderTarget(dstTexture.renderer_, dstTexture.texture_);


            SDL_Texture* surfTex = SDL_CreateTextureFromSurface(dstTexture.renderer_, surface_);
        
            SDL_Rect dst = {dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y, surface_->w, surface_->h};
            SDL_RenderCopy(dstTexture.renderer_, surfTex, nullptr, &dst);

            SDL_DestroyTexture(surfTex);
        } catch (const std::bad_cast& e) {
            std::cerr << "Bad cast in Image::drawOn: " << e.what() << '\n';
        }
    }

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; };

    void SetPixel(size_t x, size_t y, dr4::Color color) override {
        if (!surface_) return;
        if (x >= (size_t)surface_->w || y >= (size_t)surface_->h) return;
    
        SDL_LockSurface(surface_);
       
        Uint8* pixel_ptr = (Uint8*)surface_->pixels + y * surface_->pitch + x * surface_->format->BytesPerPixel;
        Uint32 mapped = SDL_MapRGBA(surface_->format, color.r, color.g, color.b, color.a);
        *(Uint32*)pixel_ptr = mapped;
        
        SDL_UnlockSurface(surface_);
    }

    dr4::Color GetPixel(size_t x, size_t y) const override {
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

    dr4::Vec2f GetSize() const override { return dr4::Vec2f(surface_->w, surface_->h); }
    float GetWidth() const override { return surface_->w; }
    float GetHeight() const override { return surface_->h; }

private:
    SDL_Surface *createSDLSurface(const int width, const int height) {
        SDL_Surface *result = SDL_CreateRGBSurfaceWithFormat(
            0,             
            width, height, 
            BIT_PER_PIXEL, 
            SDL_PIXELFORMAT_RGBA32
        );

        if (!result) {
            SDL_Log("Failed to create surface: %s", SDL_GetError());
        }

        return result;
    }
};

}
