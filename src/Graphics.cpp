#include <SDL2/SDL.h>
#include "Graphics.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/math/color.hpp"
#include "dr4/mousecodes.hpp"
#include "dr4/texture.hpp"

namespace ia {

extern "C" {
    dr4::DR4Backend* CreateDR4Backend() {
        ia::IAGraphicsBackEnd* backend = new ia::IAGraphicsBackEnd();
        return backend;
    }
}

SDL_Color convertToSDLColor(const dr4::Color &color) {
    return SDL_Color
    (
        color.r,
        color.g,
        color.b,
        color.a
    );
}

dr4::KeyMode convertToDr4KeyMode(const Uint16 SDLKeyMod) {
    uint32_t mode = dr4::KEYMOD_NONE;

    // Map SDL modifiers to dr4 modifiers
    if (SDLKeyMod & KMOD_LSHIFT) mode |= dr4::KEYMOD_LSHIFT;
    if (SDLKeyMod & KMOD_RSHIFT) mode |= dr4::KEYMOD_RSHIFT;
    if (SDLKeyMod & KMOD_LCTRL)  mode |= dr4::KEYMOD_LCTRL;
    if (SDLKeyMod & KMOD_RCTRL)  mode |= dr4::KEYMOD_RCTRL;
    if (SDLKeyMod & KMOD_LALT)   mode |= dr4::KEYMOD_LALT;
    if (SDLKeyMod & KMOD_RALT)   mode |= dr4::KEYMOD_RALT;
    if (SDLKeyMod & KMOD_CAPS)   mode |= dr4::KEYMOD_CAPS;

    return static_cast<dr4::KeyMode>(mode);
}

dr4::KeyCode convertToDr4KeyCode(const SDL_Keycode SDLKeySym) {
    switch (SDLKeySym)
    {
        // Alphabet keys
        case SDLK_a: return dr4::KEYCODE_A;
        case SDLK_b: return dr4::KEYCODE_B;
        case SDLK_c: return dr4::KEYCODE_C;
        case SDLK_d: return dr4::KEYCODE_D;
        case SDLK_e: return dr4::KEYCODE_E;
        case SDLK_f: return dr4::KEYCODE_F;
        case SDLK_g: return dr4::KEYCODE_G;
        case SDLK_h: return dr4::KEYCODE_H;
        case SDLK_i: return dr4::KEYCODE_I;
        case SDLK_j: return dr4::KEYCODE_J;
        case SDLK_k: return dr4::KEYCODE_K;
        case SDLK_l: return dr4::KEYCODE_L;
        case SDLK_m: return dr4::KEYCODE_M;
        case SDLK_n: return dr4::KEYCODE_N;
        case SDLK_o: return dr4::KEYCODE_O;
        case SDLK_p: return dr4::KEYCODE_P;
        case SDLK_q: return dr4::KEYCODE_Q;
        case SDLK_r: return dr4::KEYCODE_R;
        case SDLK_s: return dr4::KEYCODE_S;
        case SDLK_t: return dr4::KEYCODE_T;
        case SDLK_u: return dr4::KEYCODE_U;
        case SDLK_v: return dr4::KEYCODE_V;
        case SDLK_w: return dr4::KEYCODE_W;
        case SDLK_x: return dr4::KEYCODE_X;
        case SDLK_y: return dr4::KEYCODE_Y;
        case SDLK_z: return dr4::KEYCODE_Z;

        // Number keys (above letters)
        case SDLK_0: return dr4::KEYCODE_NUM0;
        case SDLK_1: return dr4::KEYCODE_NUM1;
        case SDLK_2: return dr4::KEYCODE_NUM2;
        case SDLK_3: return dr4::KEYCODE_NUM3;
        case SDLK_4: return dr4::KEYCODE_NUM4;
        case SDLK_5: return dr4::KEYCODE_NUM5;
        case SDLK_6: return dr4::KEYCODE_NUM6;
        case SDLK_7: return dr4::KEYCODE_NUM7;
        case SDLK_8: return dr4::KEYCODE_NUM8;
        case SDLK_9: return dr4::KEYCODE_NUM9;

        // Function keys
        case SDLK_F1: return dr4::KEYCODE_F1;
        case SDLK_F2: return dr4::KEYCODE_F2;
        case SDLK_F3: return dr4::KEYCODE_F3;
        case SDLK_F4: return dr4::KEYCODE_F4;
        case SDLK_F5: return dr4::KEYCODE_F5;
        case SDLK_F6: return dr4::KEYCODE_F6;
        case SDLK_F7: return dr4::KEYCODE_F7;
        case SDLK_F8: return dr4::KEYCODE_F8;
        case SDLK_F9: return dr4::KEYCODE_F9;
        case SDLK_F10: return dr4::KEYCODE_F10;
        case SDLK_F11: return dr4::KEYCODE_F11;
        case SDLK_F12: return dr4::KEYCODE_F12;
        case SDLK_F13: return dr4::KEYCODE_F13;
        case SDLK_F14: return dr4::KEYCODE_F14;
        case SDLK_F15: return dr4::KEYCODE_F15;

        // Numpad keys
        case SDLK_KP_0: return dr4::KEYCODE_NUMPAD0;
        case SDLK_KP_1: return dr4::KEYCODE_NUMPAD1;
        case SDLK_KP_2: return dr4::KEYCODE_NUMPAD2;
        case SDLK_KP_3: return dr4::KEYCODE_NUMPAD3;
        case SDLK_KP_4: return dr4::KEYCODE_NUMPAD4;
        case SDLK_KP_5: return dr4::KEYCODE_NUMPAD5;
        case SDLK_KP_6: return dr4::KEYCODE_NUMPAD6;
        case SDLK_KP_7: return dr4::KEYCODE_NUMPAD7;
        case SDLK_KP_8: return dr4::KEYCODE_NUMPAD8;
        case SDLK_KP_9: return dr4::KEYCODE_NUMPAD9;
        case SDLK_KP_PLUS: return dr4::KEYCODE_ADD;
        case SDLK_KP_MINUS: return dr4::KEYCODE_SUBTRACT;
        case SDLK_KP_MULTIPLY: return dr4::KEYCODE_MULTIPLY;
        case SDLK_KP_DIVIDE: return dr4::KEYCODE_DIVIDE;

        // Arrow keys
        case SDLK_LEFT: return dr4::KEYCODE_LEFT;
        case SDLK_RIGHT: return dr4::KEYCODE_RIGHT;
        case SDLK_UP: return dr4::KEYCODE_UP;
        case SDLK_DOWN: return dr4::KEYCODE_DOWN;

        // Modifier keys
        case SDLK_LCTRL: return dr4::KEYCODE_LCONTROL;
        case SDLK_RCTRL: return dr4::KEYCODE_RCONTROL;
        case SDLK_LSHIFT: return dr4::KEYCODE_LSHIFT;
        case SDLK_RSHIFT: return dr4::KEYCODE_RSHIFT;
        case SDLK_LALT: return dr4::KEYCODE_LALT;
        case SDLK_RALT: return dr4::KEYCODE_RALT;
        case SDLK_LGUI: return dr4::KEYCODE_LSYSTEM;
        case SDLK_RGUI: return dr4::KEYCODE_RSYSTEM;

        // Special keys
        case SDLK_ESCAPE: return dr4::KEYCODE_ESCAPE;
        case SDLK_SPACE: return dr4::KEYCODE_SPACE;
        case SDLK_RETURN: return dr4::KEYCODE_ENTER;
        case SDLK_RETURN2: return dr4::KEYCODE_ENTER;
        case SDLK_KP_ENTER: return dr4::KEYCODE_ENTER;
        case SDLK_BACKSPACE: return dr4::KEYCODE_BACKSPACE;
        case SDLK_TAB: return dr4::KEYCODE_TAB;
        case SDLK_PAGEUP: return dr4::KEYCODE_PAGEUP;
        case SDLK_PAGEDOWN: return dr4::KEYCODE_PAGEDOWN;
        case SDLK_END: return dr4::KEYCODE_END;
        case SDLK_HOME: return dr4::KEYCODE_HOME;
        case SDLK_INSERT: return dr4::KEYCODE_INSERT;
        case SDLK_DELETE: return dr4::KEYCODE_DELETE;
        case SDLK_PAUSE: return dr4::KEYCODE_PAUSE;

        // SDLKeySymbol keys
        case SDLK_LEFTBRACKET: return dr4::KEYCODE_LBRACKET;
        case SDLK_RIGHTBRACKET: return dr4::KEYCODE_RBRACKET;
        case SDLK_SEMICOLON: return dr4::KEYCODE_SEMICOLON;
        case SDLK_COMMA: return dr4::KEYCODE_COMMA;
        case SDLK_PERIOD: return dr4::KEYCODE_PERIOD;
        case SDLK_QUOTE: return dr4::KEYCODE_QUOTE;
        case SDLK_SLASH: return dr4::KEYCODE_SLASH;
        case SDLK_BACKSLASH: return dr4::KEYCODE_BACKSLASH;
        case SDLK_BACKQUOTE: return dr4::KEYCODE_TILDE;
        case SDLK_EQUALS: return dr4::KEYCODE_EQUAL;
        case SDLK_MINUS: return dr4::KEYCODE_HYPHEN;

        // Menu key
        case SDLK_MENU: return dr4::KEYCODE_MENU;
        case SDLK_APPLICATION: return dr4::KEYCODE_MENU;

        // Default for unknown keys
        default: return dr4::KEYCODE_UNKNOWN;
    }
}

dr4::MouseCode convertToDr4MouseButton(const Uint8 SDLButton) {
    switch (SDLButton) {
        case SDL_BUTTON_LEFT:   return dr4::MouseCode::MOUSECODE_LEFT;
        case SDL_BUTTON_MIDDLE: return dr4::MouseCode::MOUSECODE_MIDDLE;
        case SDL_BUTTON_RIGHT:  return dr4::MouseCode::MOUSECODE_RIGHT;
        default:                return dr4::MouseCode::MOUSECODE_UNKNOWN;
    }
}

}

dr4::Rect2f dr4::Text::GetBounds() const { 
    try {
        const ia::Font *iaFont = dynamic_cast<const ia::Font *>(font);
        int textWidth, textHeight;

        TTF_SizeText(iaFont->font_, text.c_str(), &textWidth, &textHeight);

        return dr4::Rect2f(pos.x, pos.y, textWidth, textHeight);
    } catch (const std::bad_cast& e) {
        std::cerr << "Bad cast in dr4::Text::GetBounds: " << e.what() << '\n';

        return {};
    }
}