/*
mouse enter, leave
zooom opposite of scale?
turn mods into booleans, maybe
fix the save() function of screen (high dpi)
sound change file after init?
window pixel ratio
finish key init__py
blend modes on blit
*/

#define FILE(n) sprintf(path.src+path.size,n);MOD(#n,PyUnicode_FromString(path.src))
#define NUM(n, t) MOD(n,PyLong_FromLong(t))
#define MOD(n, t) temp=t;CHECK(PyModule_AddObjectRef(program,n,temp)||PyModule_Add(self,n,temp))
#define REF(n, t) temp=t;CHECK(PyModule_AddObjectRef(program,n,temp)||PyModule_AddObjectRef(self,n,temp))
#define COLOR(r, g, b) PyTuple_Pack(3,PyFloat_FromDouble(r),PyFloat_FromDouble(g),PyFloat_FromDouble(b))
#define TYPE(e, x) CHECK(!(e.type=(PyTypeObject *)PyType_FromSpecWithBases(&e.spec,(PyObject*)x)))
#define CHECK(e) if(e)goto fail;

#include "main.h"

static Key buttons[] = {
    {SDL_BUTTON_LEFT, "left"},
    {SDL_BUTTON_MIDDLE, "middle"},
    {SDL_BUTTON_RIGHT, "right"},
    {SDL_BUTTON_X1, "x1"},
    {SDL_BUTTON_X2, "x2"}
};

static Key mods[] = {
    {SDL_KMOD_LSHIFT, "lshift"},
    {SDL_KMOD_RSHIFT, "rshift"},
    {SDL_KMOD_LEVEL5, "level_5"},
    {SDL_KMOD_LCTRL, "lctrl"},
    {SDL_KMOD_RCTRL, "rctrl"},
    {SDL_KMOD_LALT, "lalt"},
    {SDL_KMOD_RALT, "ralt"},
    {SDL_KMOD_LGUI, "lgui"},
    {SDL_KMOD_RGUI, "rgui"},
    {SDL_KMOD_NUM, "num"},
    {SDL_KMOD_CAPS, "caps"},
    {SDL_KMOD_MODE, "mode"},
    {SDL_KMOD_SCROLL, "scroll"},
    {SDL_KMOD_CTRL, "ctrl"},
    {SDL_KMOD_SHIFT, "shift"},
    {SDL_KMOD_ALT, "alt"},
    {SDL_KMOD_GUI, "gui"}
};

static Key codes[] = {
    {SDL_SCANCODE_UNKNOWN, "unknown"},
    {SDL_SCANCODE_A, "a"},
    {SDL_SCANCODE_B, "b"},
    {SDL_SCANCODE_C, "c"},
    {SDL_SCANCODE_D, "d"},
    {SDL_SCANCODE_E, "e"},
    {SDL_SCANCODE_F, "f"},
    {SDL_SCANCODE_G, "g"},
    {SDL_SCANCODE_H, "h"},
    {SDL_SCANCODE_I, "i"},
    {SDL_SCANCODE_J, "j"},
    {SDL_SCANCODE_K, "k"},
    {SDL_SCANCODE_L, "l"},
    {SDL_SCANCODE_M, "m"},
    {SDL_SCANCODE_N, "n"},
    {SDL_SCANCODE_O, "o"},
    {SDL_SCANCODE_P, "p"},
    {SDL_SCANCODE_Q, "q"},
    {SDL_SCANCODE_R, "r"},
    {SDL_SCANCODE_S, "s"},
    {SDL_SCANCODE_T, "t"},
    {SDL_SCANCODE_U, "u"},
    {SDL_SCANCODE_V, "v"},
    {SDL_SCANCODE_W, "w"},
    {SDL_SCANCODE_X, "x"},
    {SDL_SCANCODE_Y, "y"},
    {SDL_SCANCODE_Z, "z"},
    {SDL_SCANCODE_1, "_1"},
    {SDL_SCANCODE_2, "_2"},
    {SDL_SCANCODE_3, "_3"},
    {SDL_SCANCODE_4, "_4"},
    {SDL_SCANCODE_5, "_5"},
    {SDL_SCANCODE_6, "_6"},
    {SDL_SCANCODE_7, "_7"},
    {SDL_SCANCODE_8, "_8"},
    {SDL_SCANCODE_9, "_9"},
    {SDL_SCANCODE_0, "_0"},
    {SDL_SCANCODE_RETURN, "return"},
    {SDL_SCANCODE_ESCAPE, "escape"},
    {SDL_SCANCODE_BACKSPACE, "backspace"},
    {SDL_SCANCODE_TAB, "tab"},
    {SDL_SCANCODE_SPACE, "space"},
    {SDL_SCANCODE_MINUS, "minus"},
    {SDL_SCANCODE_EQUALS, "equals"},
    {SDL_SCANCODE_LEFTBRACKET, "left_bracket"},
    {SDL_SCANCODE_RIGHTBRACKET, "right_bracket"},
    {SDL_SCANCODE_BACKSLASH, "backslash"},
    {SDL_SCANCODE_NONUSHASH, "non_us_hash"},
    {SDL_SCANCODE_SEMICOLON, "semicolon"},
    {SDL_SCANCODE_APOSTROPHE, "apostrophe"},
    {SDL_SCANCODE_GRAVE, "grave"},
    {SDL_SCANCODE_COMMA, "comma"},
    {SDL_SCANCODE_PERIOD, "period"},
    {SDL_SCANCODE_SLASH, "slash"},
    {SDL_SCANCODE_CAPSLOCK, "capslock"},
    {SDL_SCANCODE_F1, "f1"},
    {SDL_SCANCODE_F2, "f2"},
    {SDL_SCANCODE_F3, "f3"},
    {SDL_SCANCODE_F4, "f4"},
    {SDL_SCANCODE_F5, "f5"},
    {SDL_SCANCODE_F6, "f6"},
    {SDL_SCANCODE_F7, "f7"},
    {SDL_SCANCODE_F8, "f8"},
    {SDL_SCANCODE_F9, "f9"},
    {SDL_SCANCODE_F10, "f10"},
    {SDL_SCANCODE_F11, "f11"},
    {SDL_SCANCODE_F12, "f12"},
    {SDL_SCANCODE_PRINTSCREEN, "print_screen"},
    {SDL_SCANCODE_SCROLLLOCK, "scroll_lock"},
    {SDL_SCANCODE_PAUSE, "pause"},
    {SDL_SCANCODE_INSERT, "insert"},
    {SDL_SCANCODE_HOME, "home"},
    {SDL_SCANCODE_PAGEUP, "page_up"},
    {SDL_SCANCODE_DELETE, "delete"},
    {SDL_SCANCODE_END, "end"},
    {SDL_SCANCODE_PAGEDOWN, "page_down"},
    {SDL_SCANCODE_RIGHT, "right"},
    {SDL_SCANCODE_LEFT, "left"},
    {SDL_SCANCODE_DOWN, "down"},
    {SDL_SCANCODE_UP, "up"},
    {SDL_SCANCODE_NUMLOCKCLEAR, "num_lock_clear"},
    {SDL_SCANCODE_KP_DIVIDE, "kp_divide"},
    {SDL_SCANCODE_KP_MULTIPLY, "kp_multiply"},
    {SDL_SCANCODE_KP_MINUS, "kp_minus"},
    {SDL_SCANCODE_KP_PLUS, "kp_plus"},
    {SDL_SCANCODE_KP_ENTER, "kp_enter"},
    {SDL_SCANCODE_KP_1, "kp_1"},
    {SDL_SCANCODE_KP_2, "kp_2"},
    {SDL_SCANCODE_KP_3, "kp_3"},
    {SDL_SCANCODE_KP_4, "kp_4"},
    {SDL_SCANCODE_KP_5, "kp_5"},
    {SDL_SCANCODE_KP_6, "kp_6"},
    {SDL_SCANCODE_KP_7, "kp_7"},
    {SDL_SCANCODE_KP_8, "kp_8"},
    {SDL_SCANCODE_KP_9, "kp_9"},
    {SDL_SCANCODE_KP_0, "kp_0"},
    {SDL_SCANCODE_KP_PERIOD, "kp_period"},
    {SDL_SCANCODE_NONUSBACKSLASH, "non_us_backslash"},
    {SDL_SCANCODE_APPLICATION, "application"},
    {SDL_SCANCODE_POWER, "power"},
    {SDL_SCANCODE_KP_EQUALS, "kp_equals"},
    {SDL_SCANCODE_F13, "f13"},
    {SDL_SCANCODE_F14, "f14"},
    {SDL_SCANCODE_F15, "f15"},
    {SDL_SCANCODE_F16, "f16"},
    {SDL_SCANCODE_F17, "f17"},
    {SDL_SCANCODE_F18, "f18"},
    {SDL_SCANCODE_F19, "f19"},
    {SDL_SCANCODE_F20, "f20"},
    {SDL_SCANCODE_F21, "f21"},
    {SDL_SCANCODE_F22, "f22"},
    {SDL_SCANCODE_F23, "f23"},
    {SDL_SCANCODE_F24, "f24"},
    {SDL_SCANCODE_EXECUTE, "execute"},
    {SDL_SCANCODE_HELP, "help"},
    {SDL_SCANCODE_MENU, "menu"},
    {SDL_SCANCODE_SELECT, "select"},
    {SDL_SCANCODE_STOP, "stop"},
    {SDL_SCANCODE_AGAIN, "again"},
    {SDL_SCANCODE_UNDO, "undo"},
    {SDL_SCANCODE_CUT, "cut"},
    {SDL_SCANCODE_COPY, "copy"},
    {SDL_SCANCODE_PASTE, "paste"},
    {SDL_SCANCODE_FIND, "find"},
    {SDL_SCANCODE_MUTE, "mute"},
    {SDL_SCANCODE_VOLUMEUP, "volume_up"},
    {SDL_SCANCODE_VOLUMEDOWN, "volume_down"},
    {SDL_SCANCODE_KP_COMMA, "kp_comma"},
    {SDL_SCANCODE_KP_EQUALSAS400, "kp_equals_400"},
    {SDL_SCANCODE_INTERNATIONAL1, "international_1"},
    {SDL_SCANCODE_INTERNATIONAL2, "international_2"},
    {SDL_SCANCODE_INTERNATIONAL3, "international_3"},
    {SDL_SCANCODE_INTERNATIONAL4, "international_4"},
    {SDL_SCANCODE_INTERNATIONAL5, "international_5"},
    {SDL_SCANCODE_INTERNATIONAL6, "international_6"},
    {SDL_SCANCODE_INTERNATIONAL7, "international_7"},
    {SDL_SCANCODE_INTERNATIONAL8, "international_8"},
    {SDL_SCANCODE_INTERNATIONAL9, "international_9"},
    {SDL_SCANCODE_LANG1, "lang_1"},
    {SDL_SCANCODE_LANG2, "lang_2"},
    {SDL_SCANCODE_LANG3, "lang_3"},
    {SDL_SCANCODE_LANG4, "lang_4"},
    {SDL_SCANCODE_LANG5, "lang_5"},
    {SDL_SCANCODE_LANG6, "lang_6"},
    {SDL_SCANCODE_LANG7, "lang_7"},
    {SDL_SCANCODE_LANG8, "lang_8"},
    {SDL_SCANCODE_LANG9, "lang_9"},
    {SDL_SCANCODE_ALTERASE, "alt_erase"},
    {SDL_SCANCODE_SYSREQ, "sysreq"},
    {SDL_SCANCODE_CANCEL, "cancel"},
    {SDL_SCANCODE_CLEAR, "clear"},
    {SDL_SCANCODE_PRIOR, "prior"},
    {SDL_SCANCODE_RETURN2, "return_2"},
    {SDL_SCANCODE_SEPARATOR, "separator"},
    {SDL_SCANCODE_OUT, "out"},
    {SDL_SCANCODE_OPER, "oper"},
    {SDL_SCANCODE_CLEARAGAIN, "clear_again"},
    {SDL_SCANCODE_CRSEL, "crsel"},
    {SDL_SCANCODE_EXSEL, "exsel"},
    {SDL_SCANCODE_KP_00, "kp_00"},
    {SDL_SCANCODE_KP_000, "kp_000"},
    {SDL_SCANCODE_THOUSANDSSEPARATOR, "thousands_separator"},
    {SDL_SCANCODE_DECIMALSEPARATOR, "decimal_separator"},
    {SDL_SCANCODE_CURRENCYUNIT, "currency_unit"},
    {SDL_SCANCODE_CURRENCYSUBUNIT, "currency_subunit"},
    {SDL_SCANCODE_KP_LEFTPAREN, "kp_left_paren"},
    {SDL_SCANCODE_KP_RIGHTPAREN, "kp_right_paren"},
    {SDL_SCANCODE_KP_LEFTBRACE, "kp_left_brace"},
    {SDL_SCANCODE_KP_RIGHTBRACE, "kp_right_brace"},
    {SDL_SCANCODE_KP_TAB, "kp_tab"},
    {SDL_SCANCODE_KP_BACKSPACE, "kp_backspace"},
    {SDL_SCANCODE_KP_A, "kp_a"},
    {SDL_SCANCODE_KP_B, "kp_b"},
    {SDL_SCANCODE_KP_C, "kp_c"},
    {SDL_SCANCODE_KP_D, "kp_d"},
    {SDL_SCANCODE_KP_E, "kp_e"},
    {SDL_SCANCODE_KP_F, "kp_f"},
    {SDL_SCANCODE_KP_XOR, "kp_xor"},
    {SDL_SCANCODE_KP_POWER, "kp_power"},
    {SDL_SCANCODE_KP_PERCENT, "kp_percent"},
    {SDL_SCANCODE_KP_LESS, "kp_less"},
    {SDL_SCANCODE_KP_GREATER, "kp_greater"},
    {SDL_SCANCODE_KP_AMPERSAND, "kp_ampersand"},
    {SDL_SCANCODE_KP_DBLAMPERSAND, "kp_dbl_ampersand"},
    {SDL_SCANCODE_KP_VERTICALBAR, "kp_vertical_bar"},
    {SDL_SCANCODE_KP_DBLVERTICALBAR, "kp_dbl_vertical_bar"},
    {SDL_SCANCODE_KP_COLON, "kp_colon"},
    {SDL_SCANCODE_KP_HASH, "kp_hash"},
    {SDL_SCANCODE_KP_SPACE, "kp_space"},
    {SDL_SCANCODE_KP_AT, "kp_at"},
    {SDL_SCANCODE_KP_EXCLAM, "kp_exclam"},
    {SDL_SCANCODE_KP_MEMSTORE, "kp_mem_store"},
    {SDL_SCANCODE_KP_MEMRECALL, "kp_mem_recall"},
    {SDL_SCANCODE_KP_MEMCLEAR, "kp_mem_clear"},
    {SDL_SCANCODE_KP_MEMADD, "kp_mem_add"},
    {SDL_SCANCODE_KP_MEMSUBTRACT, "kp_mem_subtract"},
    {SDL_SCANCODE_KP_MEMMULTIPLY, "kp_mem_multiply"},
    {SDL_SCANCODE_KP_MEMDIVIDE, "kp_mem_divide"},
    {SDL_SCANCODE_KP_PLUSMINUS, "kp_plus_minus"},
    {SDL_SCANCODE_KP_CLEAR, "kp_clear"},
    {SDL_SCANCODE_KP_CLEARENTRY, "kp_clear_entry"},
    {SDL_SCANCODE_KP_BINARY, "kp_binary"},
    {SDL_SCANCODE_KP_OCTAL, "kp_octal"},
    {SDL_SCANCODE_KP_DECIMAL, "kp_decimal"},
    {SDL_SCANCODE_KP_HEXADECIMAL, "kp_hexadecimal"},
    {SDL_SCANCODE_LCTRL, "lctrl"},
    {SDL_SCANCODE_LSHIFT, "lshift"},
    {SDL_SCANCODE_LALT, "lalt"},
    {SDL_SCANCODE_LGUI, "lgui"},
    {SDL_SCANCODE_RCTRL, "rctrl"},
    {SDL_SCANCODE_RSHIFT, "rshift"},
    {SDL_SCANCODE_RALT, "ralt"},
    {SDL_SCANCODE_RGUI, "rgui"},
    {SDL_SCANCODE_MODE, "mode"},
    {SDL_SCANCODE_SLEEP, "sleep"},
    {SDL_SCANCODE_WAKE, "wake"},
    {SDL_SCANCODE_CHANNEL_INCREMENT, "channel_increment"},
    {SDL_SCANCODE_CHANNEL_DECREMENT, "channel_decrement"},
    {SDL_SCANCODE_MEDIA_PLAY, "media_play"},
    {SDL_SCANCODE_MEDIA_PAUSE, "media_pause"},
    {SDL_SCANCODE_MEDIA_RECORD, "media_record"},
    {SDL_SCANCODE_MEDIA_FAST_FORWARD, "media_fast_forward"},
    {SDL_SCANCODE_MEDIA_REWIND, "media_rewind"},
    {SDL_SCANCODE_MEDIA_NEXT_TRACK, "media_next_track"},
    {SDL_SCANCODE_MEDIA_PREVIOUS_TRACK, "media_previous_track"},
    {SDL_SCANCODE_MEDIA_STOP, "media_stop"},
    {SDL_SCANCODE_MEDIA_EJECT, "media_eject"},
    {SDL_SCANCODE_MEDIA_PLAY_PAUSE, "media_play_pause"},
    {SDL_SCANCODE_MEDIA_SELECT, "media_select"},
    {SDL_SCANCODE_AC_NEW, "ac_new"},
    {SDL_SCANCODE_AC_OPEN, "ac_open"},
    {SDL_SCANCODE_AC_CLOSE, "ac_close"},
    {SDL_SCANCODE_AC_EXIT, "ac_exit"},
    {SDL_SCANCODE_AC_SAVE, "ac_save"},
    {SDL_SCANCODE_AC_PRINT, "ac_print"},
    {SDL_SCANCODE_AC_PROPERTIES, "ac_properties"},
    {SDL_SCANCODE_AC_SEARCH, "ac_search"},
    {SDL_SCANCODE_AC_HOME, "ac_home"},
    {SDL_SCANCODE_AC_BACK, "ac_back"},
    {SDL_SCANCODE_AC_FORWARD, "ac_forward"},
    {SDL_SCANCODE_AC_STOP, "ac_stop"},
    {SDL_SCANCODE_AC_REFRESH, "ac_refresh"},
    {SDL_SCANCODE_AC_BOOKMARKS, "ac_bookmarks"},
    {SDL_SCANCODE_SOFTLEFT, "soft_left"},
    {SDL_SCANCODE_SOFTRIGHT, "soft_right"},
    {SDL_SCANCODE_CALL, "call"},
    {SDL_SCANCODE_ENDCALL, "end_call"}
};

static Key keys[] = {
    {SDLK_UNKNOWN, "unknown"},
    {SDLK_BACKSPACE, "backspace"},
    {SDLK_TAB, "tab"},
    {SDLK_RETURN, "enter"},
    {SDLK_ESCAPE, "escape"},
    {SDLK_SPACE, "space"},
    {SDLK_EXCLAIM, "exclaim"},
    {SDLK_DBLAPOSTROPHE, "dbl_apostrophe"},
    {SDLK_HASH, "hash"},
    {SDLK_DOLLAR, "dollar"},
    {SDLK_PERCENT, "percent"},
    {SDLK_AMPERSAND, "ampersand"},
    {SDLK_APOSTROPHE, "apostrophe"},
    {SDLK_LEFTPAREN, "left_paren"},
    {SDLK_RIGHTPAREN, "right_paren"},
    {SDLK_ASTERISK, "asterisk"},
    {SDLK_PLUS, "plus"},
    {SDLK_COMMA, "comma"},
    {SDLK_MINUS, "minus"},
    {SDLK_PERIOD, "period"},
    {SDLK_SLASH, "slash"},
    {SDLK_0, "_0"},
    {SDLK_1, "_1"},
    {SDLK_2, "_2"},
    {SDLK_3, "_3"},
    {SDLK_4, "_4"},
    {SDLK_5, "_5"},
    {SDLK_6, "_6"},
    {SDLK_7, "_7"},
    {SDLK_8, "_8"},
    {SDLK_9, "_9"},
    {SDLK_COLON, "colon"},
    {SDLK_SEMICOLON, "semicolon"},
    {SDLK_LESS, "less"},
    {SDLK_EQUALS, "equals"},
    {SDLK_GREATER, "greater"},
    {SDLK_QUESTION, "question"},
    {SDLK_AT, "at"},
    {SDLK_LEFTBRACKET, "left_bracket"},
    {SDLK_BACKSLASH, "backslash"},
    {SDLK_RIGHTBRACKET, "right_bracket"},
    {SDLK_CARET, "caret"},
    {SDLK_UNDERSCORE, "underscore"},
    {SDLK_GRAVE, "grave"},
    {SDLK_A, "a"},
    {SDLK_B, "b"},
    {SDLK_C, "c"},
    {SDLK_D, "d"},
    {SDLK_E, "e"},
    {SDLK_F, "f"},
    {SDLK_G, "g"},
    {SDLK_H, "h"},
    {SDLK_I, "i"},
    {SDLK_J, "j"},
    {SDLK_K, "k"},
    {SDLK_L, "l"},
    {SDLK_M, "m"},
    {SDLK_N, "n"},
    {SDLK_O, "o"},
    {SDLK_P, "p"},
    {SDLK_Q, "q"},
    {SDLK_R, "r"},
    {SDLK_S, "s"},
    {SDLK_T, "t"},
    {SDLK_U, "u"},
    {SDLK_V, "v"},
    {SDLK_W, "w"},
    {SDLK_X, "x"},
    {SDLK_Y, "y"},
    {SDLK_Z, "z"},
    {SDLK_LEFTBRACE, "left_brace"},
    {SDLK_PIPE, "pipe"},
    {SDLK_RIGHTBRACE, "right_brace"},
    {SDLK_TILDE, "tidle"},
    {SDLK_DELETE, "delete"},
    {SDLK_PLUSMINUS, "plus_minus"},
    {SDLK_CAPSLOCK, "caps_lock"},
    {SDLK_F1, "f1"},
    {SDLK_F2, "f2"},
    {SDLK_F3, "f3"},
    {SDLK_F4, "f4"},
    {SDLK_F5, "f5"},
    {SDLK_F6, "f6"},
    {SDLK_F7, "f7"},
    {SDLK_F8, "f8"},
    {SDLK_F9, "f9"},
    {SDLK_F10, "f10"},
    {SDLK_F11, "f11"},
    {SDLK_F12, "f12"},
    {SDLK_PRINTSCREEN, "print_screen"},
    {SDLK_SCROLLLOCK, "scroll_lock"},
    {SDLK_PAUSE, "pause"},
    {SDLK_INSERT, "insert"},
    {SDLK_HOME, "home"},
    {SDLK_PAGEUP, "page_up"},
    {SDLK_END, "end"},
    {SDLK_PAGEDOWN, "page_down"},
    {SDLK_RIGHT, "right"},
    {SDLK_LEFT, "left"},
    {SDLK_DOWN, "down"},
    {SDLK_UP, "up"},
    {SDLK_NUMLOCKCLEAR, "num_lock_clear"},
    {SDLK_KP_DIVIDE, "kp_divide"},
    {SDLK_KP_MULTIPLY, "kp_multiply"},
    {SDLK_KP_MINUS, "kp_minus"},
    {SDLK_KP_PLUS, "kp_plus"},
    {SDLK_KP_ENTER, "kp_enter"},
    {SDLK_KP_1, "kp_1"},
    {SDLK_KP_2, "kp_2"},
    {SDLK_KP_3, "kp_3"},
    {SDLK_KP_4, "kp_4"},
    {SDLK_KP_5, "kp_5"},
    {SDLK_KP_6, "kp_6"},
    {SDLK_KP_7, "kp_7"},
    {SDLK_KP_8, "kp_8"},
    {SDLK_KP_9, "kp_9"},
    {SDLK_KP_0, "kp_0"},
    {SDLK_KP_PERIOD, "kp_period"},
    {SDLK_APPLICATION, "application"},
    {SDLK_POWER, "power"},
    {SDLK_KP_EQUALS, "kp_equals"},
    {SDLK_F13, "f13"},
    {SDLK_F14, "f14"},
    {SDLK_F15, "f15"},
    {SDLK_F16, "f16"},
    {SDLK_F17, "f17"},
    {SDLK_F18, "f18"},
    {SDLK_F19, "f19"},
    {SDLK_F20, "f20"},
    {SDLK_F21, "f21"},
    {SDLK_F22, "f22"},
    {SDLK_F23, "f23"},
    {SDLK_F24, "f24"},
    {SDLK_EXECUTE, "execute"},
    {SDLK_HELP, "help"},
    {SDLK_MENU, "menu"},
    {SDLK_SELECT, "select"},
    {SDLK_STOP, "stop"},
    {SDLK_AGAIN, "again"},
    {SDLK_UNDO, "undo"},
    {SDLK_CUT, "cut"},
    {SDLK_COPY, "copy"},
    {SDLK_PASTE, "paste"},
    {SDLK_FIND, "find"},
    {SDLK_MUTE, "mute"},
    {SDLK_VOLUMEUP, "volume_up"},
    {SDLK_VOLUMEDOWN, "volume_down"},
    {SDLK_KP_COMMA, "kp_comma"},
    {SDLK_KP_EQUALSAS400, "kp_equals_400"},
    {SDLK_ALTERASE, "alterase"},
    {SDLK_SYSREQ, "sys_req"},
    {SDLK_CANCEL, "cancel"},
    {SDLK_CLEAR, "clear"},
    {SDLK_PRIOR, "prior"},
    {SDLK_RETURN2, "return_2"},
    {SDLK_SEPARATOR, "separator"},
    {SDLK_OUT, "out"},
    {SDLK_OPER, "oper"},
    {SDLK_CLEARAGAIN, "clear_again"},
    {SDLK_CRSEL, "crsel"},
    {SDLK_EXSEL, "exsel"},
    {SDLK_KP_00, "kp_00"},
    {SDLK_KP_000, "kp_000"},
    {SDLK_THOUSANDSSEPARATOR, "thousands_separator"},
    {SDLK_DECIMALSEPARATOR, "decimal_separator"},
    {SDLK_CURRENCYUNIT, "currency_unit"},
    {SDLK_CURRENCYSUBUNIT, "currenct_subunit"},
    {SDLK_KP_LEFTPAREN, "kp_left_paren"},
    {SDLK_KP_RIGHTPAREN, "kp_right_paren"},
    {SDLK_KP_LEFTBRACE, "kp_left_brace"},
    {SDLK_KP_RIGHTBRACE, "kp_right_brace"},
    {SDLK_KP_TAB, "kp_tab"},
    {SDLK_KP_BACKSPACE, "kp_backspace"},
    {SDLK_KP_A, "kp_a"},
    {SDLK_KP_B, "kp_b"},
    {SDLK_KP_C, "kp_c"},
    {SDLK_KP_D, "kp_d"},
    {SDLK_KP_E, "kp_e"},
    {SDLK_KP_F, "kp_f"},
    {SDLK_KP_XOR, "kp_xor"},
    {SDLK_KP_POWER, "kp_power"},
    {SDLK_KP_PERCENT, "kp_percent"},
    {SDLK_KP_LESS, "kp_less"},
    {SDLK_KP_GREATER, "kp_greater"},
    {SDLK_KP_AMPERSAND, "kp_ampersand"},
    {SDLK_KP_DBLAMPERSAND, "kp_dbl_ampersand"},
    {SDLK_KP_VERTICALBAR, "kp_vertical_bar"},
    {SDLK_KP_DBLVERTICALBAR, "kp_dbl_vertical_bar"},
    {SDLK_KP_COLON, "kp_colon"},
    {SDLK_KP_HASH, "kp_hash"},
    {SDLK_KP_SPACE, "kp_space"},
    {SDLK_KP_AT, "kp_at"},
    {SDLK_KP_EXCLAM, "kp_exclaim"},
    {SDLK_KP_MEMSTORE, "kp_mem_store"},
    {SDLK_KP_MEMRECALL, "kp_mem_recall"},
    {SDLK_KP_MEMCLEAR, "kp_mem_clear"},
    {SDLK_KP_MEMADD, "kp_mem_add"},
    {SDLK_KP_MEMSUBTRACT, "kp_mem_subtract"},
    {SDLK_KP_MEMMULTIPLY, "kp_mem_multiply"},
    {SDLK_KP_MEMDIVIDE, "kp_mem_divide"},
    {SDLK_KP_PLUSMINUS, "kp_plus_minus"},
    {SDLK_KP_CLEAR, "kp_clear"},
    {SDLK_KP_CLEARENTRY, "kp_clear_entry"},
    {SDLK_KP_BINARY, "kp_binary"},
    {SDLK_KP_OCTAL, "kp_octal"},
    {SDLK_KP_DECIMAL, "kp_decimal"},
    {SDLK_KP_HEXADECIMAL, "kp_hexadecimal"},
    {SDLK_LCTRL, "lctrl"},
    {SDLK_LSHIFT, "lshift"},
    {SDLK_LALT, "lalt"},
    {SDLK_LGUI, "lgui"},
    {SDLK_RCTRL, "rctrl"},
    {SDLK_RSHIFT, "rshift"},
    {SDLK_RALT, "ralt"},
    {SDLK_RGUI, "rgui"},
    {SDLK_MODE, "mode"},
    {SDLK_SLEEP, "sleep"},
    {SDLK_WAKE, "wake"},
    {SDLK_CHANNEL_INCREMENT, "channel_increment"},
    {SDLK_CHANNEL_DECREMENT, "channel_decrement"},
    {SDLK_MEDIA_PLAY, "media_play"},
    {SDLK_MEDIA_PAUSE, "media_pause"},
    {SDLK_MEDIA_RECORD, "media_record"},
    {SDLK_MEDIA_FAST_FORWARD, "media_fast_forward"},
    {SDLK_MEDIA_REWIND, "media_rewind"},
    {SDLK_MEDIA_NEXT_TRACK, "media_next_track"},
    {SDLK_MEDIA_PREVIOUS_TRACK, "media_previous_track"},
    {SDLK_MEDIA_STOP, "media_stop"},
    {SDLK_MEDIA_EJECT, "media_eject"},
    {SDLK_MEDIA_PLAY_PAUSE, "media_play_pause"},
    {SDLK_MEDIA_SELECT, "media_select"},
    {SDLK_AC_NEW, "ac_new"},
    {SDLK_AC_OPEN, "ac_open"},
    {SDLK_AC_CLOSE, "ac_close"},
    {SDLK_AC_EXIT, "ac_exit"},
    {SDLK_AC_SAVE, "ac_save"},
    {SDLK_AC_PRINT, "ac_print"},
    {SDLK_AC_PROPERTIES, "ac_properties"},
    {SDLK_AC_SEARCH, "ac_search"},
    {SDLK_AC_HOME, "ac_home"},
    {SDLK_AC_BACK, "ac_back"},
    {SDLK_AC_FORWARD, "ac_forward"},
    {SDLK_AC_STOP, "ac_stop"},
    {SDLK_AC_REFRESH, "ac_refresh"},
    {SDLK_AC_BOOKMARKS, "ac_bookmarks"},
    {SDLK_SOFTLEFT, "soft_left"},
    {SDLK_SOFTRIGHT, "soft_right"},
    {SDLK_CALL, "call"},
    {SDLK_ENDCALL, "end_call"},
    {SDLK_LEFT_TAB, "left_tab"},
    {SDLK_LEVEL5_SHIFT, "level_5_shift"},
    {SDLK_MULTI_KEY_COMPOSE, "multi_key_compose"},
    {SDLK_LMETA, "lmeta"},
    {SDLK_RMETA, "rmeta"},
    {SDLK_LHYPER, "lhyper"},
    {SDLK_RHYPER, "rhyper"}
};

struct Window window;
struct Camera camera;
struct Shader shader;
struct Path path;

PyObject *error;
PyObject *program;
Texture *textures;
Font *fonts;
Audio *audio;
MIX_Mixer *mixer;

Button button[LEN(buttons)];
Button key[LEN(keys)];
Button mod[LEN(mods)];
Button code[LEN(codes)];

struct Keyboard keyboard = {
    .key = key,
    .keys = LEN(keys),
    .mod.button = mod,
    .mod.len = LEN(mods),
    .code.button = code,
    .code.len = LEN(codes)
};

struct Mouse mouse = {
    .button = button,
    .len = LEN(buttons)
};

static void clean(void) {
    SDL_GL_DestroyContext(window.ctx);
    SDL_DestroyWindow(window.sdl);

    MIX_Quit();
    SDL_Quit();
}

static int compare(uint32_t *code, Key *key) {
    return *code - key -> id;
}

static int name(Button *a, Button *b) {
    return strcmp(a -> key -> key, b -> key -> key);
}

// static void matrix(void) {
    
//     // glBindBuffer(GL_UNIFORM_BUFFER, 0);
// }

static Key *search(uint32_t code, Key *list, size_t size) {
    return bsearch(&code, list, size, sizeof(Key), (int (*)(const void *, const void *)) compare);
}

static GLuint compile(GLenum type, const GLchar *source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // GLint status;
    // glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    // if (!status) {
    //     GLchar log[512];
    //     glGetShaderInfoLog(shader, 512, NULL, log);
    //     printf("Shader compile error: %s", log);
    // }

    return shader;
}

static void create(Program *program, GLuint vert, GLuint frag) {
    glAttachShader(program -> src = glCreateProgram(), vert);
    glAttachShader(program -> src, frag);
    glLinkProgram(program -> src);
    glUniformBlockBinding(program -> src, glGetUniformBlockIndex(program -> src, "camera"), 0);

    program -> obj = glGetUniformLocation(program -> src, "object");
    program -> size = glGetUniformLocation(program -> src, "size");
    program -> color = glGetUniformLocation(program -> src, "color");
}

static void filter(Filter *filter, GLuint vert, GLuint frag) {
    glAttachShader(filter -> src = glCreateProgram(), vert);
    glAttachShader(filter -> src, frag);
    glLinkProgram(filter -> src);

    filter -> data = glGetUniformLocation(filter -> src, "data");
}

static int update(PyObject *loop) {
    INIT(PyErr_CheckSignals())
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return 1;

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            mouse.pos.x = round((event.motion.x - window.size.x / 2) * window.ratio) / window.ratio;
            mouse.pos.y = round((window.size.y / 2 - event.motion.y) * window.ratio) / window.ratio;

            mouse.move.x = event.motion.xrel;
            mouse.move.y = -event.motion.yrel;
        }

        else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            window.resize = true;
            window.size.x = event.window.data1;
            window.size.y = event.window.data2;

            if (!shader.screen && shader.active)
                glUniform2f(shader.active -> size, window.size.x, window.size.y);
        }

        else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
            if (!shader.screen)
                glViewport(0, 0, event.window.data1, event.window.data2);
        }

        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            Key *button = search(event.button.button, buttons, LEN(buttons));

            mouse.press = button -> down = button -> press = event.button.down;
            mouse.release = button -> release = !event.button.down;
        }

        else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            Key *key = search(event.key.key, keys, LEN(keys));
            Key *code = search(event.key.scancode, codes, LEN(codes));

            if (event.key.down) {
                if (event.key.mod && !event.key.repeat)
                    for (uint8_t i = 0; i < LEN(mods); i ++)
                        if (mods[i].id & event.key.mod)
                            mods[i].press = true;

                keyboard.press = code -> press = key -> press = !event.key.repeat;
                code -> repeat = key -> repeat = event.key.repeat;
                code -> down = key -> down = true;
            }

            else {
                keyboard.release = code -> release = key -> release = true;
                code -> down = key -> down = false;
            }
        }
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (loop) {
        PyObject *res = PyObject_CallObject(loop, NULL);

        INIT(!res)
        Py_DECREF(res);
    }

    for (uint16_t i = 0; i < LEN(keys); i ++)
        keys[i].press = keys[i].release = keys[i].repeat = false;

    for (uint16_t i = 0; i < LEN(codes); i ++)
        codes[i].press = codes[i].release = codes[i].repeat = false;

    for (uint8_t i = 0; i < LEN(buttons); i ++)
        buttons[i].press = buttons[i].release = false;

    for (uint8_t i = 0; i < LEN(mods); i ++)
        mods[i].press = false;

    window.resize = mouse.press = mouse.release = keyboard.press = keyboard.release = false;
    mouse.move.x = mouse.move.y = 0;
    unbind();

    return SDL_GL_SwapWindow(window.sdl) ? 0 : (PyErr_SetString(error, SDL_GetError()), -1);
}

static int module_clear(PyObject *self) {
    Py_CLEAR(program);
    Py_CLEAR(error);

    return 0;
}

#ifdef __EMSCRIPTEN__
static bool running;

EM_JS(uint32_t, width, (void), {return data.canvas.clientWidth})
EM_JS(uint32_t, height, (void), {return data.canvas.clientHeight})
EM_JS(void, init, (void), {data.open()})
EM_JS(void, end, (void), {data.close()})

static bool run(double time, PyObject *loop) {
    if (running && !update(loop))
        return true;

    Py_XDECREF(loop);
    Py_Finalize();
    chdir("/");

    return end(), false;
}
#endif

static PyObject *module_run(PyObject *self, PyObject *ignored) {
    PyObject *loop;

    if (PyObject_GetOptionalAttrString(program, "loop", &loop) >= 0) {
        if (SDL_ShowWindow(window.sdl)) {
#ifdef __EMSCRIPTEN__
            running = true;

            init();
            emscripten_request_animation_frame_loop((bool (*)(double, void *)) run, loop);

            Py_RETURN_NONE;
#else
            int status;
            while (!(status = update(loop)));

            if (status > 0) {
                Py_XDECREF(loop);
                Py_RETURN_NONE;
            }
#endif
        }

        else PyErr_SetString(error, SDL_GetError());
        Py_XDECREF(loop);
    }

    return NULL;
}

static PyObject *module_random(PyObject *self, PyObject *args) {
    double x = 0, y = 1;

    if (PyArg_ParseTuple(args, "|dd:random", &x, &y))
        return PyFloat_FromDouble(rand() / (double) RAND_MAX * fabs(y - x) + MIN(x, y));

    return NULL;
}

static PyObject *module_randint(PyObject *self, PyObject *args) {
    int x = 0, y = 1;

    if (PyArg_ParseTuple(args, "|ii:randint", &x, &y))
        return PyLong_FromLong(rand() % (abs(y - x) + 1) + MIN(x, y));

    return NULL;
}

static PyMethodDef module_methods[] = {
    {"run", module_run, METH_NOARGS, "Run the main game loop"},
    {"random", module_random, METH_VARARGS, "Generate a random floating point number"},
    {"randint", module_randint, METH_VARARGS, "Generate a random integer"},
    {NULL}
};

static int module_exec(PyObject *self) {
    printf("Welcome to JoBase\n");

    if ((error = PyErr_NewException("JoBase.SDLError", PyExc_OSError, NULL)) && !Py_AtExit(clean)) {
        if (
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) && MIX_Init() &&
            SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas") &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) &&
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0)
        ) {
#ifdef __EMSCRIPTEN__
            window.sdl = SDL_CreateWindow(NULL, width(), height(), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
#else
            window.sdl = SDL_CreateWindow(NULL, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
#endif
            if (window.sdl &&
                (mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL)) &&
                (window.ctx = SDL_GL_CreateContext(window.sdl)) &&
                (window.ratio = SDL_GetWindowPixelDensity(window.sdl)) &&
                SDL_GL_SetSwapInterval(1)
            ) {
                PyObject *temp;

                shader.active = 0;
                mouse.cursor = 0;
                shader.mode = 0;
                shader.screen = NULL;
#ifdef __EMSCRIPTEN__
                if (!(path.src = malloc(36))) {
                    PyErr_NoMemory();
                    goto fail;
                }

                path.size = 1;
                *path.src = '/';
#else
                if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
                    PyErr_SetString(PyExc_OSError, "Failed to load OpenGL");
                    goto fail;
                }

                PyObject *file = PyObject_GetAttrString(self, "__file__");

                CHECK(!file)
                const char *str = PyUnicode_AsUTF8(file);

                if (!str) {
                    Py_DECREF(file);
                    goto fail;
                }

                const char *last = strrchr(str, '/');
                path.size = (last ? last : strrchr(str, '\\')) - str + 1;

                if (!(path.src = malloc(path.size + 36))) {
                    Py_DECREF(file);
                    PyErr_NoMemory();

                    goto fail;
                }

                memcpy(path.src, str, path.size);
                Py_DECREF(file);
#endif
                CHECK(!(program = PyImport_AddModuleRef("__main__")))

                TYPE(window_data, NULL)
                TYPE(vector_data, NULL)
                TYPE(camera_data, NULL)
                TYPE(mouse_data, NULL)
                TYPE(key_data, NULL)
                TYPE(sound_data, NULL)
                TYPE(base_data, NULL)
                TYPE(button_data, NULL)
                TYPE(mod_data, NULL)
                TYPE(points_data, NULL)
                TYPE(rect_data, base_data.type)
                TYPE(shape_data, base_data.type)
                TYPE(circle_data, base_data.type)
                TYPE(screen_data, rect_data.type)
                TYPE(text_data, base_data.type)
                TYPE(line_data, shape_data.type)
                TYPE(image_data, rect_data.type)

                FILE(MAN)
                FILE(COIN)
                FILE(ENEMY)
                FILE(PICKUP)
                FILE(BLIP)

                NUM("TEXT", SDL_SYSTEM_CURSOR_TEXT)
                NUM("WAIT", SDL_SYSTEM_CURSOR_WAIT)
                NUM("CROSSHAIR", SDL_SYSTEM_CURSOR_CROSSHAIR)
                NUM("PROGRESS", SDL_SYSTEM_CURSOR_PROGRESS)
                NUM("NWSE_RESIZE", SDL_SYSTEM_CURSOR_NWSE_RESIZE)
                NUM("NESW_RESIZE", SDL_SYSTEM_CURSOR_NESW_RESIZE)
                NUM("EW_RESIZE", SDL_SYSTEM_CURSOR_EW_RESIZE)
                NUM("NS_RESIZE", SDL_SYSTEM_CURSOR_NS_RESIZE)
                NUM("MOVE", SDL_SYSTEM_CURSOR_MOVE)
                NUM("NOT_ALLOWED", SDL_SYSTEM_CURSOR_NOT_ALLOWED)
                NUM("POINTER", SDL_SYSTEM_CURSOR_POINTER)
                NUM("NW_RESIZE", SDL_SYSTEM_CURSOR_NW_RESIZE)
                NUM("N_RESIZE", SDL_SYSTEM_CURSOR_N_RESIZE)
                NUM("NE_RESIZE", SDL_SYSTEM_CURSOR_NE_RESIZE)
                NUM("E_RESIZE", SDL_SYSTEM_CURSOR_E_RESIZE)
                NUM("SE_RESIZE", SDL_SYSTEM_CURSOR_SE_RESIZE)
                NUM("S_RESIZE", SDL_SYSTEM_CURSOR_S_RESIZE)
                NUM("SW_RESIZE", SDL_SYSTEM_CURSOR_SW_RESIZE)
                NUM("W_RESIZE", SDL_SYSTEM_CURSOR_W_RESIZE)

                NUM("MULTIPLY", MULTIPLY)
                NUM("SCREEN", SCREEN)
                NUM("ADD", ADD)

                NUM("DEFAULT", 0)
                NUM("CODE", 1)
                NUM("SERIF", 2)
                NUM("DISPLAY", 3)
                NUM("PIXEL", 4)

                CHECK(!PyObject_Init((PyObject *) &keyboard.mod, mod_data.type))
                CHECK(!PyObject_Init((PyObject *) &keyboard.code, mod_data.type))

                GLfloat data[] = {-.5, .5, 0, 0, .5, .5, 1, 0, -.5, -.5, 0, 1, .5, -.5, 1, 1};
                GLuint buffers[2];

                GLuint vert_norm = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(std140) uniform camera { mat3 view; };"
                    "layout(location = 0) in vec2 vert;"

                    "uniform vec2 size;"
                    "uniform mat3 object;"

                    "void main() {"
                        "gl_Position = vec4((view * object * vec3(vert, 1)).xy * 2. / size, 0, 1);"
                    "}");

                GLuint vert_img = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(std140) uniform camera { mat3 view; };"
                    "layout(location = 0) in vec2 vert;"
                    "layout(location = 1) in vec2 coord;"

                    "uniform vec2 size;"
                    "uniform mat3 object;"
                    "out vec2 pos;"

                    "void main() {"
                        "gl_Position = vec4((view * object * vec3(vert, 1)).xy * 2. / size, 0, 1);"
                        "pos = coord;"
                    "}");

                GLuint vert_circle = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(std140) uniform camera { mat3 view; };"
                    "layout(location = 0) in vec2 vert;"

                    "uniform vec2 size;"
                    "uniform mat3 object;"
                    "out vec2 pos;"

                    "void main() {"
                        "gl_Position = vec4((view * object * vec3(vert, 1)).xy * 2. / size, 0, 1);"
                        "pos = vert;"
                    "}");

                GLuint vert_full = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(location = 0) in vec2 vert;"
                    "layout(location = 1) in vec2 coord;"

                    "out vec2 pos;"

                    "void main() {"
                        "gl_Position = vec4(vert * vec2(2, -2), 0, 1);"
                        "pos = coord;"
                    "}");

                GLuint frag_norm = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform vec4 color;"
                    "out vec4 frag;"

                    "void main() {"
                        "frag = color;"
                    "}");

                GLuint frag_img = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform sampler2D sampler;"
                    "uniform vec4 color;"

                    "in vec2 pos;"
                    "out vec4 frag;"

                    "void main() {"
                        "frag = color * texture(sampler, pos);"
                    "}");

                GLuint frag_circle = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform vec4 color;"
                    "in vec2 pos;"
                    "out vec4 frag;"

                    "void main() {"
                        "frag = length(pos) < .5 ? color : vec4(0);"
                    "}");

                GLuint frag_text = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform sampler2D sampler;"
                    "uniform vec4 color;"

                    "in vec2 pos;"
                    "out vec4 frag;"

                    "void main() {"
                        "vec3 tex = texture(sampler, pos).rgb;"
                        "float dist = max(min(tex.r, tex.g), min(max(tex.r, tex.g), tex.b));"
                        "float width = fwidth(dist);"
                        "float opacity = smoothstep(.5 - width, .5 + width, dist);"

                        //dist - .2 light
                        //dist + .2 bold

                        "frag = color * vec4(1, 1, 1, opacity);"
                        // "frag = texture(sampler, pos);"
                    "}");

                GLuint frag_warp = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform sampler2D sampler;"
                    "uniform vec2 data;"

                    "in vec2 pos;"
                    "out vec4 frag;"

                    // "vec4 blend(vec4 a, vec4 b, int mode) {"
                    //     "vec4 res = b;"

                    //     "res = mix(res, a + b, float(mode == " STR(ADDITIVE) "));"
                    //     "res = mix(res, a * b, float(mode == " STR(MULTIPLY) "));"
                    //     "res = mix(res, 1. - (1. - a) * (1. - b), float(mode == " STR(SCREEN) "));"

                    //     "vec4 overlay = vec4("
                    //         "a.r < .5 ? (2. * a.r * b.r) : (1. - 2. * (1. - a.r) * (1. - b.r)),"
                    //         "a.g < .5 ? (2. * a.g * b.g) : (1. - 2. * (1. - a.g) * (1. - b.g)),"
                    //         "a.b < .5 ? (2. * a.b * b.b) : (1. - 2. * (1. - a.b) * (1. - b.b)), 1.);"

                    //     "return mix(res, overlay, float(mode == " STR(OVERLAY) "));"
                    // "}"

                    "void main() {"
                        "vec2 scale = vec2(textureSize(sampler, 0)) / data.x;"
                        "vec2 st = (pos - .5) * scale;"
                        "float len = length(st);"

                        "if (len < 1.) {"
                            "float theta = atan(st.y, st.x);"
                            "float rad = pow(len, abs(data.y));"

                            "st.x = rad * cos(theta);"
                            "st.y = rad * sin(theta);"

                            "st = st / scale + .5;"
                        "} else {"
                            "st = pos;"
                        "}"

                        "frag = texture(sampler, st);"
                    "}");

                create(&shader.plain, vert_norm, frag_norm);
                create(&shader.image, vert_img, frag_img);
                create(&shader.circle, vert_circle, frag_circle);
                create(&shader.text, vert_img, frag_text);
                filter(&shader.warp, vert_full, frag_warp);

                glDeleteShader(vert_norm);
                glDeleteShader(vert_img);
                glDeleteShader(vert_circle);
                glDeleteShader(vert_full);
                glDeleteShader(frag_norm);
                glDeleteShader(frag_img);
                glDeleteShader(frag_circle);
                glDeleteShader(frag_text);
                glDeleteShader(frag_warp);

                glGenVertexArrays(1, &shader.vao);
                glGenBuffers(2, buffers);

                glBindVertexArray(shader.vao);
                glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
                glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *) (sizeof(GLfloat) * 2));
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);

                glBindVertexArray(0);
                glDeleteBuffers(1, &buffers[0]);

                glBindBuffer(GL_UNIFORM_BUFFER, shader.ubo = buffers[1]);
                glBufferData(GL_UNIFORM_BUFFER, 48, NULL, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, shader.ubo);

                glActiveTexture(GL_TEXTURE0);
                glEnable(GL_BLEND);
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                for (uint16_t i = 0; i < LEN(keys); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &key[i], button_data.type))
                    key[i].key = &keys[i];
                }

                for (uint16_t i = 0; i < LEN(codes); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &code[i], button_data.type))
                    code[i].key = &codes[i];
                }

                for (uint8_t i = 0; i < LEN(mods); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &mod[i], button_data.type))
                    mod[i].key = &mods[i];
                }

                for (uint8_t i = 0; i < LEN(buttons); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &button[i], button_data.type))
                    button[i].key = &buttons[i];
                }

                MOD("camera", PyObject_CallObject((PyObject *) camera_data.type, NULL))
                MOD("window", PyObject_CallObject((PyObject *) window_data.type, NULL))
                MOD("mouse", PyObject_CallObject((PyObject *) mouse_data.type, NULL))
                MOD("key", PyObject_CallObject((PyObject *) key_data.type, NULL))
                REF("Rect", (PyObject *) rect_data.type)
                REF("Shape", (PyObject *) shape_data.type)
                REF("Line", (PyObject *) line_data.type)
                REF("Image", (PyObject *) image_data.type)
                REF("Circle", (PyObject *) circle_data.type)
                REF("Text", (PyObject *) text_data.type)
                REF("Sound", (PyObject *) sound_data.type)
                REF("Screen", (PyObject *) screen_data.type)

                MOD("WHITE", COLOR(1, 1, 1))
                MOD("BLACK", COLOR(0, 0, 0))
                MOD("GRAY", COLOR(.5, .5, .5))
                MOD("DARK_GRAY", COLOR(.2, .2, .2))
                MOD("LIGHT_GRAY", COLOR(.8, .8, .8))
                MOD("BROWN", COLOR(.6, .2, .2))
                MOD("TAN", COLOR(.8, .7, .6))
                MOD("RED", COLOR(1, 0, 0))
                MOD("DARK_RED", COLOR(.6, 0, 0))
                MOD("SALMON", COLOR(1, .5, .5))
                MOD("ORANGE", COLOR(1, .5, 0))
                MOD("GOLD", COLOR(1, .8, 0))
                MOD("YELLOW", COLOR(1, 1, 0))
                MOD("OLIVE", COLOR(.5, .5, 0))
                MOD("LIME", COLOR(0, 1, 0))
                MOD("DARK_GREEN", COLOR(0, .4, 0))
                MOD("GREEN", COLOR(0, .5, 0))
                MOD("AQUA", COLOR(0, 1, 1))
                MOD("BLUE", COLOR(0, 0, 1))
                MOD("LIGHT_BLUE", COLOR(.5, .8, 1))
                MOD("AZURE", COLOR(.9, 1, 1))
                MOD("NAVY", COLOR(0, 0, .5))
                MOD("PURPLE", COLOR(.5, 0, 1))
                MOD("PINK", COLOR(1, .75, .8))
                MOD("MAGENTA", COLOR(1, 0, 1))

                qsort(keys, LEN(keys), sizeof(Key), (int (*)(const void *, const void *)) compare);
                // qsort(mods, LEN(mods), sizeof(Key), (int (*)(const void *, const void *)) compare);
                qsort(codes, LEN(codes), sizeof(Key), (int (*)(const void *, const void *)) compare);
                qsort(key, LEN(keys), sizeof(Button), (int (*)(const void *, const void *)) name);
                qsort(mod, LEN(mods), sizeof(Button), (int (*)(const void *, const void *)) name);
                qsort(code, LEN(code), sizeof(Button), (int (*)(const void *, const void *)) name);

                return PyModule_AddFunctions(program, module_methods);

            fail:
                Py_DECREF(error);
                Py_XDECREF(program);

                return -1;
            }
        }

        PyErr_SetString(error, SDL_GetError());
        Py_DECREF(error);
    }

    return -1;
}

static int module_traverse(PyObject *self, visitproc visit, void *arg) {
    Py_VISIT(program);
    Py_VISIT(error);

    return 0;
}

static void module_free(void *closure) {
    module_clear(NULL);

    free(window.title);
    free(path.src);

    while (textures) {
        Texture *this = textures;

        textures = this -> next;
        glDeleteTextures(1, &this -> src);

        free(this -> name);
        free(this);
    }

    while (fonts) {
        Font *this = fonts;

        fonts = this -> next;
        glDeleteTextures(1, &this -> src);

        free(this -> chars);
        free(this);
    }

    while (audio) {
        Audio *this = audio;

        audio = this -> next;
        MIX_DestroyAudio(this -> src);

        free(this -> name);
        free(this);
    }

    for (uint8_t i = 0; i < SDL_SYSTEM_CURSOR_COUNT; i ++)
        if (mouse.cursors[i])
            SDL_DestroyCursor(mouse.cursors[i]);

    if (shader.vao) {
        glDeleteVertexArrays(1, &shader.vao);
        glDeleteBuffers(1, &shader.ubo);

        glDeleteProgram(shader.plain.src);
        glDeleteProgram(shader.image.src);
        glDeleteProgram(shader.circle.src);
        glDeleteProgram(shader.text.src);
        glDeleteProgram(shader.warp.src);
    }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void stop(void) {
    running = false;
}

EMSCRIPTEN_KEEPALIVE
void call(const char *string) {
    Py_Initialize();
    chdir("/main");

    PyObject *root = PyUnicode_FromString("/main");

    if (PyList_Append(PySys_GetObject("path"), root))
        Py_DECREF(root);

    else {
        Py_DECREF(root);

        if (!string) {
            FILE *file = fopen("/main/__main__.py", "r");

            if (file && !PyRun_SimpleFileEx(file, "__main__.py", 1) && running)
                return;
        }

        else if (!PyRun_SimpleString(string) && running)
            return;
    }

    Py_Finalize();
    chdir("/");
}
#endif

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, module_exec},
    {0, NULL}
};

static PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "JoBase",
    .m_size = 0,
    .m_slots = module_slots,
    .m_methods = module_methods,
    .m_traverse = module_traverse,
    .m_clear = module_clear,
    .m_free = module_free
};

PyMODINIT_FUNC PyInit_JoBase(void) {
    return srand(time(NULL)), PyModuleDef_Init(&module);
}