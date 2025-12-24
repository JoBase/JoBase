/*
TODO:

Base - top, bottom, left, right
- Constants (colours)
- Check OpenGL binding to zero
- Check all mallocs and strdup
- mousedown
mouse enter, leave
- keydown
- text.units
update __init__.pyi
more fonts
- images memory leak
- sdl errors
zooom opposite of scale?
turn mods into booleans, maybe
- sound files (init name = DEFAULT) <-- actually no
- add blit() to all shapes
fix the save() function of screen
sound change file after init?
- line transparency
window pixel ratio
- sound default audio
finish collision (and docs)
docs: text doesn not instance rect
pyargparse for screen

TO Test:

screen saving
*/

#define FILE(n, s) sprintf(path.src+path.size,s);CHECK(PyModule_AddStringConstant(program,n,path.src))
#define ADD(n, t) CHECK(PyModule_Add(program,n,t))
#define COLOR(r, g, b) PyTuple_Pack(3,PyFloat_FromDouble(r),PyFloat_FromDouble(g),PyFloat_FromDouble(b))
#define TYPE(e, x) CHECK(!(e.type=(PyTypeObject *)PyType_FromSpecWithBases(&e.spec,x)))
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
    {SDLK_RIGHTBRACKET, "righ_bracket"},
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

struct Keyboard keyboard = {
    .key = key,
    .mod = mod,
    .mods = LEN(mods),
    .keys = LEN(keys)
};

struct Mouse mouse = {
    .button = button,
    .len = LEN(buttons)
};

static void clean(void) {
    printf("FREE stuff\n");

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

static Key *search(uint32_t code, Key *list, size_t size) {
    return bsearch(&code, list, size, sizeof(Key), (int (*)(const void *, const void *)) compare);
}

static GLuint compile(GLenum type, const GLchar *source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    return shader;
}

static void create(struct Program *program, GLuint vertex, GLuint fragment) {
    program -> src = glCreateProgram();

    glAttachShader(program -> src, vertex);
    glAttachShader(program -> src, fragment);

    glLinkProgram(program -> src);
    glUniformBlockBinding(program -> src, glGetUniformBlockIndex(program -> src, "camera"), 0);

    program -> obj = glGetUniformLocation(program -> src, "object");
    program -> color = glGetUniformLocation(program -> src, "color");
}

static int update(PyObject *loop) {
    INIT(PyErr_CheckSignals())
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return 1;

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            mouse.pos.x = event.motion.x - window.size.x / 2;
            mouse.pos.y = window.size.y / 2 - event.motion.y;

            mouse.move.x = event.motion.xrel;
            mouse.move.y = event.motion.yrel;
        }

        else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            window.resize = true;
            window.size.x = event.window.data1;
            window.size.y = event.window.data2;
        }

        else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            glViewport(0, 0, event.window.data1, event.window.data2);

        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            Key *button = search(event.button.button, buttons, LEN(buttons));

            mouse.press = button -> down = button -> press = event.button.down;
            mouse.release = button -> release = !event.button.down;
        }

        else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            Key *key = search(event.key.key, keys, LEN(keys));

            if (event.key.mod)
                search(event.key.mod, mods, LEN(mods)) -> press = true;

            if (event.key.down) {
                keyboard.press = key -> press = !event.key.repeat;
                key -> repeat = event.key.repeat;
                key -> down = true;
            }

            else {
                keyboard.release = key -> release = true;
                key -> down = false;
            }
        }
    }

    const double sx = 2 / window.size.x / camera.scale.x;
    const double sy = 2 / window.size.y / camera.scale.y;

    GLfloat matrix[] = {
        sx, 0, 0, 0,
        0, sy, 0, 0,
        -camera.pos.x * sx, -camera.pos.y * sy, -1, 0
    };

    glBindBuffer(GL_UNIFORM_BUFFER, shader.ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof matrix, matrix);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (loop) {
        PyObject *res = PyObject_CallObject(loop, NULL);

        INIT(!res)
        Py_DECREF(res);
    }

    for (uint16_t i = 0; i < LEN(keys); i ++)
        keys[i].press = keys[i].release = keys[i].repeat = false;

    for (uint8_t i = 0; i < LEN(mods); i ++)
        mods[i].press = false;

    for (uint8_t i = 0; i < LEN(buttons); i ++)
        buttons[i].press = false;

    window.resize = mouse.press = mouse.release = keyboard.press = keyboard.release = false;
    mouse.move.x = mouse.move.y = 0;

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
        return PyFloat_FromDouble(rand() / (RAND_MAX / fabs(y - x)) + MIN(x, y));

    return NULL;
}

static PyMethodDef module_methods[] = {
    {"run", module_run, METH_NOARGS, "Run the main game loop"},
    {"random", module_random, METH_VARARGS, "Generate a random floating point number"},
    {NULL}
};

static int module_exec(PyObject *self) {
    printf("Welcome to JoBase\n");

    if ((error = PyErr_NewException("JoBase.SDLError", PyExc_OSError, NULL)) && !Py_AtExit(clean)) {
        if (
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) && MIX_Init() &&
            SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas") &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0) &&
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
#ifdef __EMSCRIPTEN__
                path.size = 0;

                if (!(path.src = malloc(36))) {
                    PyErr_NoMemory();
                    goto fail;
                }
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
                TYPE(rect_data, (PyObject *) base_data.type)
                TYPE(shape_data, (PyObject *) base_data.type)
                TYPE(circle_data, (PyObject *) base_data.type)
                TYPE(screen_data, (PyObject *) rect_data.type)
                TYPE(text_data, (PyObject *) base_data.type)
                TYPE(line_data, (PyObject *) shape_data.type)
                TYPE(image_data, (PyObject *) rect_data.type)

                FILE("MAN", "images/man.png")
                FILE("COIN", "images/coin.png")
                FILE("ENEMY", "images/enemy.png")
                FILE("PICKUP", "audio/pickup.wav")
                FILE("BLIP", "audio/blip.wav")

                CHECK(PyModule_AddIntConstant(program, "DEFAULT", 0))
                CHECK(PyModule_AddIntConstant(program, "CODE", 1))
                CHECK(!PyObject_Init(&keyboard.map, mod_data.type))

                for (uint16_t i = 0; i < LEN(keys); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &key[i], button_data.type))
                    key[i].key = &keys[i];
                }

                for (uint8_t i = 0; i < LEN(mods); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &mod[i], button_data.type))
                    mod[i].key = &mods[i];
                }

                for (uint8_t i = 0; i < LEN(buttons); i ++) {
                    CHECK(!PyObject_Init((PyObject *) &button[i], button_data.type))
                    button[i].key = &buttons[i];
                }

                ADD("camera", PyObject_CallObject((PyObject *) camera_data.type, NULL))
                ADD("window", PyObject_CallObject((PyObject *) window_data.type, NULL))
                ADD("mouse", PyObject_CallObject((PyObject *) mouse_data.type, NULL))
                ADD("key", PyObject_CallObject((PyObject *) key_data.type, NULL))
                ADD("Rect", (PyObject *) rect_data.type)
                ADD("Shape", (PyObject *) shape_data.type)
                ADD("Line", (PyObject *) line_data.type)
                ADD("Image", (PyObject *) image_data.type)
                ADD("Circle", (PyObject *) circle_data.type)
                ADD("Text", (PyObject *) text_data.type)
                ADD("Sound", (PyObject *) sound_data.type)
                ADD("Screen", (PyObject *) screen_data.type)

                ADD("WHITE", COLOR(1, 1, 1))
                ADD("BLACK", COLOR(0, 0, 0))
                ADD("GRAY", COLOR(.5, .5, .5))
                ADD("DARK_GRAY", COLOR(.2, .2, .2))
                ADD("LIGHT_GRAY", COLOR(.8, .8, .8))
                ADD("BROWN", COLOR(.6, .2, .2))
                ADD("TAN", COLOR(.8, .7, .6))
                ADD("RED", COLOR(1, 0, 0))
                ADD("DARK_RED", COLOR(.6, 0, 0))
                ADD("SALMON", COLOR(1, .5, .5))
                ADD("ORANGE", COLOR(1, .5, 0))
                ADD("GOLD", COLOR(1, .8, 0))
                ADD("YELLOW", COLOR(1, 1, 0))
                ADD("OLIVE", COLOR(.5, .5, 0))
                ADD("LIME", COLOR(0, 1, 0))
                ADD("DARK_GREEN", COLOR(0, .4, 0))
                ADD("GREEN", COLOR(0, .5, 0))
                ADD("AQUA", COLOR(0, 1, 1))
                ADD("BLUE", COLOR(0, 0, 1))
                ADD("LIGHT_BLUE", COLOR(.5, .8, 1))
                ADD("AZURE", COLOR(.9, 1, 1))
                ADD("NAVY", COLOR(0, 0, .5))
                ADD("PURPLE", COLOR(.5, 0, 1))
                ADD("PINK", COLOR(1, .75, .8))
                ADD("MAGENTA", COLOR(1, 0, 1))

                qsort(keys, LEN(keys), sizeof(Key), (int (*)(const void *, const void *)) compare);
                qsort(mods, LEN(mods), sizeof(Key), (int (*)(const void *, const void *)) compare);
                qsort(key, LEN(keys), sizeof(Button), (int (*)(const void *, const void *)) name);
                qsort(mod, LEN(mods), sizeof(Button), (int (*)(const void *, const void *)) name);

                GLfloat data[] = {-.5, .5, 0, 0, .5, .5, 1, 0, -.5, -.5, 0, 1, .5, -.5, 1, 1};
                GLuint buffers[2];

                GLuint vn = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(std140) uniform camera { mat3 view; };"
                    "layout(location = 0) in vec2 vert;"

                    "uniform mat3 object;"

                    "void main() {"
                        "gl_Position = vec4(view * object * vec3(vert, 1), 1);"
                    "}");

                GLuint fn = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform vec4 color;"
                    "out vec4 frag;"

                    "void main() {"
                        "frag = color;"
                    "}");

                GLuint vi = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(std140) uniform camera { mat3 view; };"
                    "layout(location = 0) in vec2 vert;"
                    "layout(location = 1) in vec2 coord;"

                    "uniform mat3 object;"
                    "out vec2 pos;"

                    "void main() {"
                        "gl_Position = vec4(view * object * vec3(vert, 1), 1);"
                        "pos = coord;"
                    "}");

                GLuint fi = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform sampler2D sampler;"
                    "uniform vec4 color;"

                    "in vec2 pos;"
                    "out vec4 frag;"

                    "void main() {"
                        "frag = color * texture(sampler, pos);"
                    "}");

                GLuint vc = compile(GL_VERTEX_SHADER,
                    "#version " VERSION "\n"

                    "layout(std140) uniform camera { mat3 view; };"
                    "layout(location = 0) in vec2 vert;"

                    "uniform mat3 object;"
                    "out vec2 pos;"

                    "void main() {"
                        "gl_Position = vec4(view * object * vec3(vert, 1), 1);"
                        "pos = vert;"
                    "}");

                GLuint fc = compile(GL_FRAGMENT_SHADER,
                    "#version " VERSION "\n"
                    "precision mediump float;"

                    "uniform vec4 color;"
                    "in vec2 pos;"
                    "out vec4 frag;"

                    "void main() {"
                        "frag = length(pos) < .5 ? color : vec4(0);"
                    "}");

                GLuint ft = compile(GL_FRAGMENT_SHADER,
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

                        "frag = color * vec4(1, 1, 1, opacity);"
                        // "frag = texture(sampler, pos);"
                    "}");

                create(&shader.plain, vn, fn);
                create(&shader.image, vi, fi);
                create(&shader.circle, vc, fc);
                create(&shader.text, vi, ft);

                glDeleteShader(vn);
                glDeleteShader(fn);
                glDeleteShader(vi);
                glDeleteShader(fi);
                glDeleteShader(fc);
                glDeleteShader(ft);

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
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                return PyModule_AddFunctions(program, module_methods);

            fail:
                printf("failool\n");
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
        MIX_DestroyAudio(audio -> src);

        free(this -> name);
        free(this);
    }

    printf("FREE vao\n");

    if (shader.vao) {
        glDeleteVertexArrays(1, &shader.vao);
        glDeleteBuffers(1, &shader.ubo);

        glDeleteProgram(shader.plain.src);
        glDeleteProgram(shader.image.src);
        glDeleteProgram(shader.circle.src);
        glDeleteProgram(shader.text.src);
    }

    printf("FREE end\n");
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
    .m_traverse = module_traverse,
    .m_clear = module_clear,
    .m_free = module_free
};

PyMODINIT_FUNC PyInit_JoBase(void) {
    return srand(time(NULL)), PyModuleDef_Init(&module);
}