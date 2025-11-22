#pragma once

#include <cassert>
#include <string>
#include <optional>

#include "IAError.hpp"
#include "dr4/texture.hpp"
#include "dr4/math/rect.hpp"
#include "Common.hpp"
#include "SDLRAII.hpp"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct SDL_RWops;
struct TTF_Font;
struct SDL_Color;

namespace ia {

class Window;          
class RendererGuard; 

// ---------------- Texture ----------------
class Texture : public dr4::Texture {
    const Window&              window_;
    ia::raii::SDL_Texture      texture_;
    dr4::Vec2f                 pos_;
    dr4::Vec2f                 zero_;
    std::optional<SDL_Rect>    clipRect_;

    friend class Line;
    friend class Circle;
    friend class Rectangle;
    friend class Text;
    friend class Image;
    friend class Window;

public:
    Texture(const Window &window, int width = 100, int height = 100);
    ~Texture() override;

    void DrawOn(dr4::Texture& texture) const override;
    void SetPos(dr4::Vec2f pos) override;
    dr4::Vec2f GetPos() const override;

    void SetSize(dr4::Vec2f size) override;
    dr4::Vec2f GetSize() const override;
    float GetWidth() const override;
    float GetHeight() const override;

    void SetZero(dr4::Vec2f pos) override;
    dr4::Vec2f GetZero() const override;

    void SetClipRect(dr4::Rect2f rect) override;
    void RemoveClipRect() override;
    dr4::Rect2f GetClipRect() const override;

    void Clear(dr4::Color color) override;

    const Window &getWindow() const;
    const ia::raii::SDL_Renderer &getRenderer() const;
};

// ---------------- Line ----------------
class Line : public dr4::Line {
    dr4::Vec2f start_;
    dr4::Vec2f end_;
    float thickness_;
    SDL_Color color_;

public:
    Line() = default;
    Line(dr4::Vec2f start, dr4::Vec2f end, float thickness, SDL_Color color);
    ~Line() override = default;

    void DrawOn(dr4::Texture &texture) const override;

    void SetPos(dr4::Vec2f pos) override;
    dr4::Vec2f GetPos() const override;

    void SetStart(dr4::Vec2f start) override;
    void SetEnd(dr4::Vec2f end) override;
    void SetColor(dr4::Color color) override;
    void SetThickness(float thickness) override;

    dr4::Vec2f GetStart() const override;
    dr4::Vec2f GetEnd() const override;
    dr4::Color GetColor() const override;
    float GetThickness() const override;
};

// ---------------- Circle ----------------
class Circle : public dr4::Circle {
    dr4::Vec2f pos_;
    dr4::Vec2f radius_;
    float borderThickness_;
    SDL_Color fillColor_;
    SDL_Color borderColor_;

public:
    Circle() = default;
    Circle(dr4::Vec2f pos, dr4::Vec2f radius, float borderThickness,
           SDL_Color fillColor, SDL_Color borderColor);
    ~Circle() override = default;

    void DrawOn(dr4::Texture &texture) const override;

    void SetPos(dr4::Vec2f pos) override;
    dr4::Vec2f GetPos() const override;

    void SetCenter(dr4::Vec2f center) override;
    void SetRadius(dr4::Vec2f radius) override;
    void SetFillColor(dr4::Color color) override;
    void SetBorderColor(dr4::Color color) override;
    void SetBorderThickness(float thickness) override;

    dr4::Vec2f GetCenter() const override;
    dr4::Vec2f GetRadius() const override;
    dr4::Color GetFillColor() const override;
    dr4::Color GetBorderColor() const override;
    float GetBorderThickness() const override;
};

// ---------------- Rectangle ----------------
class Rectangle : public dr4::Rectangle {
    dr4::Rect2f rect_;
    float borderThickness_;
    SDL_Color fillColor_;
    SDL_Color borderColor_;

public:
    Rectangle() = default;
    Rectangle(dr4::Vec2f pos, dr4::Vec2f size, float borderThickness,
              SDL_Color fillColor, SDL_Color borderColor);
    ~Rectangle() override = default;

    void DrawOn(dr4::Texture& texture) const override;

    void SetPos(dr4::Vec2f pos) override;
    dr4::Vec2f GetPos() const override;

    void SetSize(dr4::Vec2f size) override;
    void SetFillColor(dr4::Color color) override;
    void SetBorderThickness(float thickness) override;
    void SetBorderColor(dr4::Color color) override;

    dr4::Vec2f GetSize() const override;
    dr4::Color GetFillColor() const override;
    float GetBorderThickness() const override;
    dr4::Color GetBorderColor() const override;
};

// ---------------- Font ----------------
class Font : public dr4::Font {
    static constexpr int DEFAULT_FONT_SIZE = 24;

    int fontSize_ = DEFAULT_FONT_SIZE;
    raii::TTF_Font font_;
    std::optional<std::string> lastFileLoadpath;
    raii::SDL_RWops lastLoadBufer;

    friend class Text;

public:
    Font();
    ~Font() override;

    void LoadFromFile(const std::string& path) override;
    void LoadFromBuffer(const void *buffer, size_t size) override;

    float GetAscent(float fontSize) const override;
    float GetDescent(float fontSize) const override;

    float getFontSize() const;
    void setFontSize(float fontSize);

private:
    void resetFont();
    void loadFromRWBUffer(raii::SDL_RWops bufer);
    raii::TTF_Font getFontCopy(float fontSize) const;
};

// ---------------- FontGuard ----------------
struct FontGuard {
    Font *font_;
    float savedFontSize_;

    explicit FontGuard(Font *font);
    ~FontGuard();
};

// ---------------- Text ----------------
class Text : public dr4::Text {
    static constexpr float DEFAULT_FONT_SIZE = 24;
    
    float fontSize_ = DEFAULT_FONT_SIZE;
    Font *font_;
    
    SDL_Color color_ = SDL_Color{0,0,0,255};

    std::string text_ = "Text";
    dr4::Text::VAlign vAlign_ = dr4::Text::VAlign::TOP;
    dr4::Vec2f pos_{};

public:
    Text() = default;
    ~Text() override = default;

    void DrawOn(dr4::Texture& texture) const override;
    void SetPos(dr4::Vec2f pos) override;
    dr4::Vec2f GetPos() const override;

    void SetText(const std::string &text) override;
    void SetColor(dr4::Color color) override;
    void SetFontSize(float size) override;
    void SetVAlign(dr4::Text::VAlign align) override;
    void SetFont(const dr4::Font *font) override;

    dr4::Vec2f         GetBounds() const override;
    const std::string &GetText() const override;
    dr4::Color         GetColor() const override;
    float              GetFontSize() const override;
    VAlign             GetVAlign() const override;
    const Font        *GetFont() const override;

private:
    void DrawTextDetail(const raii::SDL_Renderer &renderer, 
                        Font *font, const char* text,
                        int x, int y, VAlign valign, SDL_Color color) const;
};

// ---------------- Image ----------------
class Image : public dr4::Image {
    static constexpr int BIT_PER_PIXEL = 32;

    dr4::Vec2f pos_;
    raii::SDL_Surface surface_;

public:
    Image();
    Image(int width, int height);
    ~Image() override;

    void DrawOn(dr4::Texture &texture) const override;

    void SetPos(dr4::Vec2f pos) override;
    dr4::Vec2f GetPos() const override;

    void SetPixel(size_t x, size_t y, dr4::Color color) override;
    dr4::Color GetPixel(size_t x, size_t y) const override;

    void SetSize(dr4::Vec2f size) override;
    dr4::Vec2f GetSize() const override;
    float GetWidth() const override;
    float GetHeight() const override;

private:
    raii::SDL_Surface createSDLSurface(int width, int height);
};

} // namespace ia
