#include "Drawable.hpp"
#include "Window.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include <iostream>
#include <utility>

namespace ia {

// ---------------- Texture ----------------
Texture::Texture(const Window &window, int width, int height):
    window_(window), texture_(nullptr), pos_{0,0}, zero_{0,0}, clipRect_(std::nullopt)
{
    if (width > 0 && height > 0) {
        texture_ = ia::raii::SDL_CreateTexture(window_.getRenderer(),
                                               SDL_PIXELFORMAT_RGBA8888,
                                               SDL_TEXTUREACCESS_TARGET,
                                               width, height);
        requireSDLCondition(texture_ != nullptr);
    
        requireSDLCondition(SDL_SetTextureBlendMode(texture_.get(), SDL_BLENDMODE_BLEND) == 0);
        requireSDLCondition(SDL_SetTextureAlphaMod(texture_.get(), 255) == 0);
    } else {
        throw_invalid_argument("width/height must be positive");
    }
}

Texture::~Texture() = default;

void Texture::DrawOn(dr4::Texture& texture) const {
    try {
        const Texture &dstTexture = dynamic_cast<const Texture &>(texture);
        assert(texture_ && dstTexture.texture_);

        RendererGuard renderGuard(dstTexture.getRenderer());
        requireSDLCondition(SDL_SetRenderTarget(dstTexture.getRenderer().get(), dstTexture.texture_.get()) == 0);

        int textureWidth, textureHeight;
        requireSDLCondition(SDL_QueryTexture(texture_.get(), NULL, NULL, &textureWidth, &textureHeight) == 0);
        
        SDL_Rect dstClipRect = convertToSDLRect(dstTexture.GetClipRect());
        dstClipRect.x += GetZero().x;
        dstClipRect.y += GetZero().y;
        requireSDLCondition(SDL_RenderSetClipRect(dstTexture.getRenderer().get(), &dstClipRect) == 0);
    
        SDL_Rect dstRect = 
        {
            static_cast<int>(dstTexture.zero_.x + pos_.x),
            static_cast<int>(dstTexture.zero_.y + pos_.y),
            textureWidth,
            textureHeight
        };
        requireSDLCondition(SDL_RenderCopy(dstTexture.getRenderer().get(), texture_.get(), nullptr, &dstRect) == 0);
    } catch (const std::bad_cast&) {
        std::throw_with_nested(Dr4Exception("Bad cast in Texture::DrawOn"));
    }
}

void Texture::SetPos(dr4::Vec2f pos) { pos_ = pos; }
dr4::Vec2f Texture::GetPos() const { return pos_; }

void Texture::SetSize(dr4::Vec2f size) {
    raii::SDL_Texture newTexture = raii::SDL_CreateTexture(window_.getRenderer(),
                                                           SDL_PIXELFORMAT_RGBA8888,
                                                           SDL_TEXTUREACCESS_TARGET,
                                                           static_cast<int>(size.x), static_cast<int>(size.y));
    requireSDLCondition(newTexture != nullptr);
    requireSDLCondition(SDL_SetTextureBlendMode(newTexture.get(), SDL_BLENDMODE_BLEND) == 0);
    requireSDLCondition(SDL_SetTextureAlphaMod(newTexture.get(), 255) == 0);

    if (texture_) texture_.reset();
    texture_.swap(newTexture);
}

dr4::Vec2f Texture::GetSize() const {
    int textureWidth = 0, textureHeight = 0;
    requireSDLCondition(SDL_QueryTexture(texture_.get(), NULL, NULL, &textureWidth, &textureHeight) == 0);
    return dr4::Vec2f{static_cast<float>(textureWidth), static_cast<float>(textureHeight)};
}

float Texture::GetWidth() const { return GetSize().x; }
float Texture::GetHeight() const { return GetSize().y; }

void Texture::SetZero(dr4::Vec2f pos) { zero_ = pos; }
dr4::Vec2f Texture::GetZero() const { return zero_; }

void Texture::SetClipRect(dr4::Rect2f rect) {
    clipRect_ = SDL_Rect
    (
        static_cast<int>(rect.pos.x),
        static_cast<int>(rect.pos.y),
        static_cast<int>(rect.size.x),
        static_cast<int>(rect.size.y)
    );
}

void Texture::RemoveClipRect() { clipRect_.reset(); }

dr4::Rect2f Texture::GetClipRect() const {
    return convertToDr4Rect(clipRect_.value_or(SDL_Rect(0, 0, GetWidth(), GetHeight())));
}

void Texture::Clear(dr4::Color color) {
    RendererGuard renderGuard(window_.getRenderer()); 
    requireSDLCondition(SDL_SetRenderTarget(window_.getRenderer().get(), texture_.get()) == 0);
    requireSDLCondition(SDL_SetRenderDrawColor(getRenderer().get(), color.r, color.g, color.b, color.a) == 0);    
    requireSDLCondition(SDL_RenderClear(getRenderer().get()) == 0);
}

const Window &Texture::getWindow() const { return window_; }
const ia::raii::SDL_Renderer &Texture::getRenderer() const { return window_.getRenderer(); }

// ---------------- Line ----------------
Line::Line(dr4::Vec2f start, dr4::Vec2f end, float thickness, SDL_Color color)
    : start_(start), end_(end), thickness_(thickness), color_(color) {}

void Line::DrawOn(dr4::Texture &texture) const {
    try {
        const Texture &dstTexture = dynamic_cast<const Texture &>(texture);

        RendererGuard renderGuard(dstTexture.getRenderer());
        requireSDLCondition(SDL_SetRenderTarget(dstTexture.getRenderer().get(), dstTexture.texture_.get()) == 0);

        SDL_Rect dstClipRect = convertToSDLRect(dstTexture.GetClipRect());
        dstClipRect.x += dstTexture.GetZero().x;
        dstClipRect.y += dstTexture.GetZero().y;
        requireSDLCondition(SDL_RenderSetClipRect(dstTexture.getRenderer().get(), &dstClipRect) == 0);

        thickLineColor(dstTexture.getRenderer().get(),
                       dstTexture.zero_.x + start_.x,
                       dstTexture.zero_.y + start_.y,
                       dstTexture.zero_.x + end_.x,
                       dstTexture.zero_.y + end_.y,
                       thickness_, SDLColorToGfxColor(color_));
    } catch (const std::bad_cast&) {
        std::throw_with_nested(Dr4Exception("Bad cast in Line::DrawOn"));
    }
}

void Line::SetPos(dr4::Vec2f pos) {
    dr4::Vec2f delta = end_ - start_;
    start_ = pos;
    end_ = pos + delta;
}
dr4::Vec2f Line::GetPos() const { return start_; }
void Line::SetStart(dr4::Vec2f start) { start_ = start; }
void Line::SetEnd(dr4::Vec2f end) { end_ = end; }
void Line::SetColor(dr4::Color color) { color_ = convertToSDLColor(color); }
void Line::SetThickness(float thickness) { thickness_ = thickness; }
dr4::Vec2f Line::GetStart() const { return start_; }
dr4::Vec2f Line::GetEnd() const { return end_; }
dr4::Color Line::GetColor() const { return dr4::Color(color_.r, color_.g, color_.b, color_.a); }
float Line::GetThickness() const { return thickness_; }

// ---------------- Circle ----------------
Circle::Circle(dr4::Vec2f pos, dr4::Vec2f radius, float borderThickness,
               SDL_Color fillColor, SDL_Color borderColor)
    : pos_(pos), radius_(radius), borderThickness_(borderThickness),
      fillColor_(fillColor), borderColor_(borderColor) {}

void Circle::DrawOn(dr4::Texture &texture) const {
    try {
        const Texture &dstTexture = dynamic_cast<const Texture &>(texture);

        RendererGuard renderGuard(dstTexture.getRenderer());
        requireSDLCondition(SDL_SetRenderTarget(dstTexture.getRenderer().get(), dstTexture.texture_.get()) == 0);

        SDL_Rect dstClipRect = convertToSDLRect(dstTexture.GetClipRect());
        dstClipRect.x += dstTexture.GetZero().x;
        dstClipRect.y += dstTexture.GetZero().y;
        requireSDLCondition(SDL_RenderSetClipRect(dstTexture.getRenderer().get(), &dstClipRect) == 0);

        if (borderThickness_ <= 0) {
            requireSDLCondition(filledEllipseRGBA(dstTexture.getRenderer().get(),
                dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y,
                radius_.x, radius_.y, fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a) == 0);
            return;
        }

        dr4::Vec2f innerRadius = radius_ - dr4::Vec2f(borderThickness_, borderThickness_);
        if (innerRadius.x <= 0 || innerRadius.y <= 0) {
            requireSDLCondition(filledEllipseRGBA(dstTexture.getRenderer().get(),
                dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y,
                radius_.x, radius_.y, borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a) == 0);
            return;
        }

        requireSDLCondition(SDL_SetRenderDrawBlendMode(dstTexture.getRenderer().get(), SDL_BLENDMODE_BLEND) == 0);

        requireSDLCondition(filledEllipseRGBA(dstTexture.getRenderer().get(),
            dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y,
            radius_.x, radius_.y, borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a) == 0);

        requireSDLCondition(SDL_SetRenderDrawBlendMode(dstTexture.getRenderer().get(), SDL_BLENDMODE_NONE) == 0);

        requireSDLCondition(filledEllipseRGBA(dstTexture.getRenderer().get(),
            dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y,
            innerRadius.x, innerRadius.y, fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a) == 0);

        requireSDLCondition(SDL_SetRenderDrawBlendMode(dstTexture.getRenderer().get(), SDL_BLENDMODE_BLEND) == 0);
    } catch (const std::bad_cast&) {
        std::throw_with_nested(Dr4Exception("Bad cast in Circle::DrawOn"));
    }
}

void Circle::SetPos(dr4::Vec2f pos) { pos_ = pos; }
dr4::Vec2f Circle::GetPos() const { return pos_; }
void Circle::SetCenter(dr4::Vec2f center) { pos_ = center; }
void Circle::SetRadius(dr4::Vec2f radius) { radius_ = radius; }
void Circle::SetFillColor(dr4::Color color) { fillColor_ = convertToSDLColor(color); }
void Circle::SetBorderColor(dr4::Color color) { borderColor_ = convertToSDLColor(color); }
void Circle::SetBorderThickness(float thickness) { borderThickness_ = thickness; }
dr4::Vec2f Circle::GetCenter() const { return pos_; }
dr4::Vec2f Circle::GetRadius() const { return radius_; }
dr4::Color Circle::GetFillColor() const { return convertToDr4Color(fillColor_); }
dr4::Color Circle::GetBorderColor() const { return convertToDr4Color(borderColor_); }
float Circle::GetBorderThickness() const { return borderThickness_; }

// ---------------- Rectangle ----------------
Rectangle::Rectangle(dr4::Vec2f pos, dr4::Vec2f size, float borderThickness,
                     SDL_Color fillColor, SDL_Color borderColor)
    : rect_(pos, size), borderThickness_(borderThickness), fillColor_(fillColor), borderColor_(borderColor) {}

void Rectangle::DrawOn(dr4::Texture& texture) const {
    try {
        const Texture &dstTexture = dynamic_cast<const Texture &>(texture);

        RendererGuard renderGuard(dstTexture.getRenderer());
        requireSDLCondition(SDL_SetRenderTarget(dstTexture.getRenderer().get(), dstTexture.texture_.get()) == 0);

        SDL_Rect dstClipRect = convertToSDLRect(dstTexture.GetClipRect());
        dstClipRect.x += dstTexture.GetZero().x;
        dstClipRect.y += dstTexture.GetZero().y;
        requireSDLCondition(SDL_RenderSetClipRect(dstTexture.getRenderer().get(), &dstClipRect) == 0);

        if (2 * borderThickness_ >= std::fmin(rect_.size.x, rect_.size.y)) {
            requireSDLCondition(SDL_SetRenderDrawColor(
                dstTexture.getRenderer().get(), borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a) == 0);

            SDL_Rect innerRect = convertToSDLRect(rect_);
            innerRect.x += dstTexture.zero_.x;
            innerRect.y += dstTexture.zero_.y;

            requireSDLCondition(SDL_RenderFillRect(dstTexture.getRenderer().get(), &innerRect) == 0);
            return;
        }

        SDL_Rect innerRect = SDL_Rect {
            static_cast<int>(dstTexture.zero_.x + rect_.pos.x + borderThickness_),
            static_cast<int>(dstTexture.zero_.y + rect_.pos.y + borderThickness_),
            static_cast<int>(rect_.size.x - 2 * borderThickness_),
            static_cast<int>(rect_.size.y - 2 * borderThickness_)
        };

        requireSDLCondition(SDL_SetRenderDrawColor(dstTexture.getRenderer().get(), fillColor_.r, fillColor_.g, fillColor_.b, fillColor_.a) == 0);
        requireSDLCondition(SDL_RenderFillRect(dstTexture.getRenderer().get(), &innerRect) == 0);

        requireSDLCondition(SDL_SetRenderDrawColor(dstTexture.getRenderer().get(), borderColor_.r, borderColor_.g, borderColor_.b, borderColor_.a) == 0);

        SDL_Rect top {
            static_cast<int>(dstTexture.zero_.x + rect_.pos.x),
            static_cast<int>(dstTexture.zero_.y + rect_.pos.y),
            static_cast<int>(rect_.size.x),
            static_cast<int>(borderThickness_)
        };
        requireSDLCondition(SDL_RenderFillRect(dstTexture.getRenderer().get(), &top) == 0);

        SDL_Rect bottom {
            static_cast<int>(dstTexture.zero_.x + rect_.pos.x),
            static_cast<int>(dstTexture.zero_.y + rect_.pos.y + rect_.size.y - borderThickness_),
            static_cast<int>(rect_.size.x),
            static_cast<int>(borderThickness_)
        };
        requireSDLCondition(SDL_RenderFillRect(dstTexture.getRenderer().get(), &bottom) == 0);

        SDL_Rect left {
            static_cast<int>(dstTexture.zero_.x + rect_.pos.x),
            static_cast<int>(dstTexture.zero_.y + rect_.pos.y + borderThickness_),
            static_cast<int>(borderThickness_),
            static_cast<int>(rect_.size.y - 2 * borderThickness_)
        };
        requireSDLCondition(SDL_RenderFillRect(dstTexture.getRenderer().get(), &left) == 0);

        SDL_Rect right {
            static_cast<int>(dstTexture.zero_.x + rect_.pos.x + rect_.size.x - borderThickness_),
            static_cast<int>(dstTexture.zero_.y + rect_.pos.y + borderThickness_),
            static_cast<int>(borderThickness_),
            static_cast<int>(rect_.size.y - 2 * borderThickness_)
        };
        requireSDLCondition(SDL_RenderFillRect(dstTexture.getRenderer().get(), &right) == 0);
    } catch (const std::bad_cast&) {
        std::throw_with_nested(Dr4Exception("Bad cast in Rectangle::DrawOn"));
    }
}

void Rectangle::SetPos(dr4::Vec2f pos) { rect_.pos = pos; }
dr4::Vec2f Rectangle::GetPos() const { return rect_.pos; }
void Rectangle::SetSize(dr4::Vec2f size) { rect_.size = size; }
void Rectangle::SetFillColor(dr4::Color color) { fillColor_ = convertToSDLColor(color); }
void Rectangle::SetBorderThickness(float thickness) { borderThickness_ = thickness; }
void Rectangle::SetBorderColor(dr4::Color color) { borderColor_ = convertToSDLColor(color); }
dr4::Vec2f Rectangle::GetSize() const { return rect_.size; }
dr4::Color Rectangle::GetFillColor() const { return convertToDr4Color(fillColor_); }
float Rectangle::GetBorderThickness() const { return borderThickness_; }
dr4::Color Rectangle::GetBorderColor() const { return convertToDr4Color(borderColor_); }

// ---------------- Font ----------------
Font::Font() = default;
Font::~Font() = default;

void Font::resetFont() {
    font_.reset();
    lastLoadBufer.reset();
}

void Font::LoadFromFile(const std::string& path) {
    resetFont();

    lastFileLoadpath = path;
    std::string fontPath = lastFileLoadpath.value_or(std::string{});
    
    font_ = raii::TTF_OpenFont(fontPath.c_str(), fontSize_);
    requireTTFCondition(font_ != nullptr);
}

void Font::LoadFromBuffer(const void *buffer, size_t size) {
    assert(buffer);
    resetFont();

    lastLoadBufer = raii::SDL_RWFromConstMem(buffer, size);
    requireSDLCondition(lastLoadBufer != nullptr);
    font_ = raii::TTF_OpenFontRW(lastLoadBufer.get(), 0, fontSize_);
    if (!font_) {
        resetFont();
        throw TTFException();
    }
}

float Font::GetAscent(float fontSize) const {
    setFontSizeDetail(fontSize);
    float fontAscent = static_cast<float>(TTF_FontAscent(font_.get()));
    setFontSizeDetail(fontSize_);

    return fontAscent;
}

float Font::GetDescent(float fontSize) const {
    setFontSizeDetail(fontSize);
    float fontDescent = static_cast<float>(TTF_FontDescent(font_.get()));
    setFontSizeDetail(fontSize_);

    return fontDescent;
}

float Font::getFontSize() const { return static_cast<float>(fontSize_); }

void Font::setFontSizeDetail(float fontSize) const {
    requireTTFCondition(TTF_SetFontSize(font_.get(), fontSize) == 0);    
}

void Font::setFontSize(float fontSize) {
    requireTTFCondition(TTF_SetFontSize(font_.get(), fontSize) == 0);    
}

// ---------------- FontGuard ----------------
FontGuard::FontGuard(Font *font): font_(font), savedFontSize_(font ? font->getFontSize() : 0) {
    assert(font);
}
FontGuard::~FontGuard() { font_->setFontSize(savedFontSize_); }

// ---------------- Text ----------------
void Text::DrawOn(dr4::Texture& texture) const {
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
        RendererGuard renderGuard(dstTexture.getRenderer());
        FontGuard fontGuard(font_);

        requireSDLCondition(SDL_SetRenderTarget(dstTexture.getRenderer().get(), dstTexture.texture_.get()) == 0);

        SDL_Rect dstClipRect = convertToSDLRect(dstTexture.GetClipRect());
        dstClipRect.x += dstTexture.GetZero().x;
        dstClipRect.y += dstTexture.GetZero().y;
        requireSDLCondition(SDL_RenderSetClipRect(dstTexture.getRenderer().get(), &dstClipRect) == 0);

        DrawTextDetail(dstTexture.getRenderer(), font_, text_.c_str(),
                       dstTexture.zero_.x + pos_.x, dstTexture.zero_.y + pos_.y, vAlign_, color_);
    } catch (const std::bad_cast&) {
        std::throw_with_nested(Dr4Exception("Bad cast in Text::DrawOn"));
    }
}

void Text::SetPos(dr4::Vec2f pos) { pos_ = pos; }
dr4::Vec2f Text::GetPos() const { return pos_; }

void Text::SetText(const std::string &text) { text_ = text; }
void Text::SetColor(dr4::Color color) { color_ = convertToSDLColor(color); }
void Text::SetFontSize(float size) { fontSize_ = size; }
void Text::SetVAlign(dr4::Text::VAlign align) { vAlign_ = align; }

void Text::SetFont(const dr4::Font *font) {
    auto f = dynamic_cast<const Font*>(font);
    if (!f) { std::throw_with_nested(Dr4Exception("Bad cast in Text::SetFont")); }
    font_ = const_cast<Font *>(f);
}

dr4::Vec2f Text::GetBounds() const {
    FontGuard fontGuard(font_);
    font_->setFontSize(fontSize_);
    int textWidth = 0, textHeight = 0;
    requireTTFCondition(TTF_SizeUTF8(font_->font_.get(), text_.c_str(), &textWidth, &textHeight) == 0);
    return dr4::Vec2f(static_cast<float>(textWidth), static_cast<float>(textHeight));
}

const std::string &Text::GetText() const { return text_; }
dr4::Color Text::GetColor() const { return convertToDr4Color(color_); }
float Text::GetFontSize() const { return fontSize_; }
Text::VAlign Text::GetVAlign() const { return vAlign_; }
const Font *Text::GetFont() const { return font_; }

void Text::DrawTextDetail(const raii::SDL_Renderer &renderer, 
                          Font *font, const char* text,
                          int x, int y, VAlign valign, SDL_Color color) const
{
    font->setFontSize(fontSize_);

    raii::SDL_Surface surf = raii::TTF_RenderUTF8_Blended(font->font_, text, color);
    if (!surf) surf = raii::TTF_RenderUTF8_Blended(font->font_, " ", color); 
    requireSDLCondition(surf != nullptr);

    raii::SDL_Texture tex = raii::SDL_CreateTextureFromSurface(renderer, surf);
    requireSDLCondition(tex != nullptr);

    int w = 0, h = 0;
    requireSDLCondition(SDL_QueryTexture(tex.get(), NULL, NULL, &w, &h) == 0);

    SDL_Rect dst = { x, y, w, h };

    switch (valign) {
        case VAlign::TOP:        break;
        case VAlign::MIDDLE:     dst.y -= h / 2; break;
        case VAlign::BASELINE:   dst.y -= TTF_FontAscent(font->font_.get()); break;
        case VAlign::BOTTOM:     dst.y -= h; break;
        default: break;
    }

    requireSDLCondition(SDL_RenderCopy(renderer.get(), tex.get(), nullptr, &dst) == 0);
}

// ---------------- Image ----------------
Image::Image() {
    surface_ = createSDLSurface(100, 100);
}

Image::Image(int width, int height) {
    surface_ = createSDLSurface(width, height);
}

Image::~Image() = default;

void Image::DrawOn(dr4::Texture &texture) const try {
    const Texture &dstTexture = dynamic_cast<const Texture &>(texture);

    RendererGuard renderGuard(dstTexture.getRenderer());
    requireSDLCondition(SDL_SetRenderTarget(dstTexture.getRenderer().get(), dstTexture.texture_.get()) == 0);

    SDL_Rect dstClipRect = convertToSDLRect(dstTexture.GetClipRect());
    dstClipRect.x += dstTexture.GetZero().x;
    dstClipRect.y += dstTexture.GetZero().y;
    requireSDLCondition(SDL_RenderSetClipRect(dstTexture.getRenderer().get(), &dstClipRect) == 0);

    raii::SDL_Texture surfTex = raii::SDL_CreateTextureFromSurface(dstTexture.getRenderer(), surface_);
    requireSDLCondition(surfTex != nullptr);

    SDL_Rect dst = {
        static_cast<int>(dstTexture.zero_.x + pos_.x),
        static_cast<int>(dstTexture.zero_.y + pos_.y),
        surface_->w,
        surface_->h
    };
    requireSDLCondition(SDL_RenderCopy(dstTexture.getRenderer().get(), surfTex.get(), nullptr, &dst) == 0);
} catch (const std::bad_cast&) { std::throw_with_nested(Dr4Exception("Bad cast in Image::DrawOn")); }

void Image::SetPos(dr4::Vec2f pos) { pos_ = pos; }
dr4::Vec2f Image::GetPos() const { return pos_; }

void Image::SetPixel(size_t x, size_t y, dr4::Color color) {
    if (!surface_) return;
    if (x >= static_cast<size_t>(surface_->w) || y >= static_cast<size_t>(surface_->h)) return;

    SDL_LockSurface(surface_.get());

    Uint8* pixel_ptr = static_cast<Uint8*>(surface_->pixels) + y * surface_->pitch + x * surface_->format->BytesPerPixel;
    Uint32 mapped = SDL_MapRGBA(surface_->format, color.r, color.g, color.b, color.a);
    *reinterpret_cast<Uint32*>(pixel_ptr) = mapped;

    SDL_UnlockSurface(surface_.get());
}

dr4::Color Image::GetPixel(size_t x, size_t y) const {
    assert(surface_->format->BytesPerPixel == 4);
    assert(sizeof(dr4::Color) == 4);

    Uint8* pixel_ptr = static_cast<Uint8*>(surface_->pixels)
                        + y * surface_->pitch
                        + x * (surface_->format->BytesPerPixel);

    return *reinterpret_cast<dr4::Color*>(pixel_ptr);
}

void Image::SetSize(dr4::Vec2f size) {
    assert(surface_);
    raii::SDL_Surface newSurface = createSDLSurface(static_cast<int>(size.x), static_cast<int>(size.y));
    requireSDLCondition(newSurface != nullptr);
    surface_ = std::move(newSurface);
}

dr4::Vec2f Image::GetSize() const { return dr4::Vec2f{static_cast<float>(surface_->w), static_cast<float>(surface_->h)}; }
float Image::GetWidth() const { return static_cast<float>(surface_->w); }
float Image::GetHeight() const { return static_cast<float>(surface_->h); }

raii::SDL_Surface Image::createSDLSurface(int width, int height) {
    raii::SDL_Surface result = raii::SDL_CreateRGBSurfaceWithFormat(
        0,
        width, height,
        BIT_PER_PIXEL,
        SDL_PIXELFORMAT_RGBA32
    );

    requireSDLCondition(result != nullptr);
    return result;
}

} // namespace ia
