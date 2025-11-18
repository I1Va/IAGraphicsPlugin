#include "SDLRAII.hpp"
#include "Window.hpp"

namespace ia::raii
{

SDL_Window SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags) {
    assert(title);

    ::SDL_Window* raw = ::SDL_CreateWindow(title, x, y, w, h, flags);
    return SDL_Window(raw);
}

SDL_Renderer SDL_CreateRenderer(const SDL_Window &w, int index, Uint32 flags) {
    assert(w != nullptr);

    ::SDL_Renderer* raw = SDL_CreateRenderer(w.get(), index, flags);
    return SDL_Renderer(raw);
}

SDL_Texture SDL_CreateTexture(const SDL_Renderer &renderer, Uint32 format, int access, int w, int h) {
    assert(renderer != nullptr);
    ::SDL_Texture* raw = ::SDL_CreateTexture(renderer.get(), format, access, w, h);
    return SDL_Texture(raw);
}

SDL_Surface  TTF_RenderUTF8_Blended(const TTF_Font &font, const char* text, SDL_Color color) {
    assert(font);
    assert(text);

    ::SDL_Surface* raw = ::TTF_RenderUTF8_Blended(font.get(), text, color);

    return SDL_Surface(raw);
}

TTF_Font TTF_OpenFont(const char* file, int ptsize) {
    assert(file);
    ::TTF_Font* raw = ::TTF_OpenFont(file, ptsize);
    return TTF_Font(raw);
}

TTF_Font TTF_OpenFontRW(::SDL_RWops* src, int freesrc, int ptsize) {
    assert(src);
    ::TTF_Font* raw = ::TTF_OpenFontRW(src, freesrc, ptsize);
    return TTF_Font(raw);
}

SDL_RWops SDL_RWFromConstMem(const void* buffer, size_t size) {
    assert(buffer);
    ::SDL_RWops* raw = ::SDL_RWFromConstMem(buffer, static_cast<int>(size));
    return SDL_RWops(raw);
}

SDL_Texture SDL_CreateTextureFromSurface(const SDL_Renderer &renderer, const SDL_Surface &surface) {
    assert(renderer);
    assert(surface);

    ::SDL_Texture* raw = ::SDL_CreateTextureFromSurface(renderer.get(), surface.get());
    return SDL_Texture(raw);
}

SDL_Surface SDL_CreateRGBSurfaceWithFormat(int flags, int width, int height, int depth, Uint32 format) {
    ::SDL_Surface* raw = ::SDL_CreateRGBSurfaceWithFormat(flags, width, height, depth, format);
    return SDL_Surface(raw);
}

}
