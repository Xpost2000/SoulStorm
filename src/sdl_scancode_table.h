#ifndef SDL_SCANCODE_TABLE_H
#define SDL_SCANCODE_TABLE_H

local int translate_sdl_scancode(int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_ESCAPE:    return KEY_ESCAPE;
        case SDL_SCANCODE_1:         return KEY_1;
        case SDL_SCANCODE_2:         return KEY_2;
        case SDL_SCANCODE_3:         return KEY_3;
        case SDL_SCANCODE_4:         return KEY_4;
        case SDL_SCANCODE_5:         return KEY_5;
        case SDL_SCANCODE_6:         return KEY_6;
        case SDL_SCANCODE_7:         return KEY_7;
        case SDL_SCANCODE_8:         return KEY_8;
        case SDL_SCANCODE_9:         return KEY_9;
        case SDL_SCANCODE_0:         return KEY_0;
        case SDL_SCANCODE_MINUS:     return KEY_MINUS;
        case SDL_SCANCODE_EQUALS:    return KEY_EQUALS;
        case SDL_SCANCODE_BACKSPACE: return KEY_BACKSPACE;

        case SDL_SCANCODE_TAB: return KEY_TAB;

        case SDL_SCANCODE_Q:            return KEY_Q;
        case SDL_SCANCODE_W:            return KEY_W;
        case SDL_SCANCODE_E:            return KEY_E;
        case SDL_SCANCODE_R:            return KEY_R;
        case SDL_SCANCODE_T:            return KEY_T;
        case SDL_SCANCODE_Y:            return KEY_Y;
        case SDL_SCANCODE_U:            return KEY_U;
        case SDL_SCANCODE_I:            return KEY_I;
        case SDL_SCANCODE_O:            return KEY_O;
        case SDL_SCANCODE_P:            return KEY_P;
        case SDL_SCANCODE_LEFTBRACKET:  return KEY_LEFT_BRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return KEY_RIGHT_BRACKET;
        case SDL_SCANCODE_RETURN:       return KEY_RETURN;

        case SDL_SCANCODE_LCTRL: return KEY_CTRL;
        case SDL_SCANCODE_RCTRL: return KEY_CTRL;

        case SDL_SCANCODE_A: return KEY_A;
        case SDL_SCANCODE_S: return KEY_S;
        case SDL_SCANCODE_D: return KEY_D;
        case SDL_SCANCODE_F: return KEY_F;
        case SDL_SCANCODE_G: return KEY_G;
        case SDL_SCANCODE_H: return KEY_H;
        case SDL_SCANCODE_J: return KEY_J;
        case SDL_SCANCODE_K: return KEY_K;
        case SDL_SCANCODE_L: return KEY_L;
        case SDL_SCANCODE_SEMICOLON: return KEY_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return KEY_QUOTE;
        case SDL_SCANCODE_GRAVE: return KEY_BACKQUOTE;

        case SDL_SCANCODE_LSHIFT: return KEY_SHIFT; // left shift
        case SDL_SCANCODE_RSHIFT: return KEY_SHIFT; // left shift
        case SDL_SCANCODE_BACKSLASH: return KEY_BACKSLASH;

        case SDL_SCANCODE_Z:      return KEY_Z;
        case SDL_SCANCODE_X:      return KEY_X;
        case SDL_SCANCODE_C:      return KEY_C;
        case SDL_SCANCODE_V:      return KEY_V;
        case SDL_SCANCODE_B:      return KEY_B;
        case SDL_SCANCODE_N:      return KEY_N;
        case SDL_SCANCODE_M:      return KEY_M;
        case SDL_SCANCODE_COMMA:  return KEY_COMMA;
        case SDL_SCANCODE_PERIOD: return KEY_PERIOD;
        case SDL_SCANCODE_SLASH:  return KEY_FORWARDSLASH;

        case SDL_SCANCODE_PRINTSCREEN: return KEY_PRINTSCREEN;
        case SDL_SCANCODE_LALT:        return KEY_ALT;
        case SDL_SCANCODE_RALT:        return KEY_ALT;
        case SDL_SCANCODE_SPACE:       return KEY_SPACE;

        case SDL_SCANCODE_F1:  return KEY_F1;
        case SDL_SCANCODE_F2:  return KEY_F2;
        case SDL_SCANCODE_F3:  return KEY_F3;
        case SDL_SCANCODE_F4:  return KEY_F4;
        case SDL_SCANCODE_F5:  return KEY_F5;
        case SDL_SCANCODE_F6:  return KEY_F6;
        case SDL_SCANCODE_F7:  return KEY_F7;
        case SDL_SCANCODE_F8:  return KEY_F8;
        case SDL_SCANCODE_F9:  return KEY_F9;
        case SDL_SCANCODE_F10: return KEY_F10;
        case SDL_SCANCODE_F11: return KEY_F11;
        case SDL_SCANCODE_F12: return KEY_F12;

        case SDL_SCANCODE_NUMLOCKCLEAR: return KEY_NUMBER_LOCK;
        case SDL_SCANCODE_SCROLLLOCK:   return KEY_SCROLL_LOCK;


        case SDL_SCANCODE_PAGEUP:   return KEY_PAGEUP;
        case SDL_SCANCODE_HOME:     return KEY_HOME;
        case SDL_SCANCODE_PAGEDOWN: return KEY_PAGEDOWN;
        case SDL_SCANCODE_INSERT:   return KEY_INSERT;
        case SDL_SCANCODE_DELETE:   return KEY_DELETE;
        case SDL_SCANCODE_END:      return KEY_END;

        case SDL_SCANCODE_UP:    return KEY_UP;
        case SDL_SCANCODE_DOWN:  return KEY_DOWN;
        case SDL_SCANCODE_LEFT:  return KEY_LEFT;
        case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
        
    }

    return -1;
}
#endif
