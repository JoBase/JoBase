#define DEL(e, n) if(!e)return PyErr_SetString(PyExc_AttributeError,"Cannot delete the '"n"' attribute"),-1;
#define INIT(e) if(e)return-1;
#define ERR(e) ((e)==-1&&PyErr_Occurred())
#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)
#define LEN(e) sizeof e/sizeof*e

#define MAN "images/man.png"
#define COIN "images/coin.png"
#define ENEMY "images/enemy.png"
#define PICKUP "audio/pickup.wav"
#define BLIP "audio/blip.wav"

#define BLUR 1
#define WARP 2
#define _USE_MATH_DEFINES

#ifdef __EMSCRIPTEN__
#define VERSION "300 es"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

extern uint32_t width(void);
extern uint32_t height(void);
#else
#define VERSION "330 core"
#include "glad/glad.h"
#endif

#include <Python.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <tesselator.h>
#include <stb_image.h>
#include <stb_image_write.h>

#if PY_VERSION_HEX < 0x30a0000 // 3.10
static inline PyObject *_Py_NewRef(PyObject *e) {
    return Py_INCREF(e), e;
}

#define Py_NewRef(e) _Py_NewRef((PyObject*)e)
#endif

#if PY_VERSION_HEX < 0x30d0000 // 3.13
static inline int PyObject_GetOptionalAttrString(PyObject *obj, const char *name, PyObject **res) {
    return (*res = PyObject_GetAttrString(obj, name)) ? 1 : PyErr_ExceptionMatches(PyExc_AttributeError) ? (PyErr_Clear(), 0) : -1;
}

static inline PyObject *PyImport_AddModuleRef(const char *name) {
    return Py_NewRef(PyImport_AddModule(name));
}

static inline int PyModule_Add(PyObject *module, const char *name, PyObject *value) {
    if (PyModule_AddObject(module, name, value)) {
        Py_XDECREF(value);
        return -1;
    }

    return 0;
}

#define PyLong_AsInt(e) (int)PyLong_AsLong(e)
#endif

enum {x, y, z, w};
enum {r, g, b, a};

typedef struct Vec2 Vec2;
typedef struct Vec3 Vec3;
typedef struct Vec4 Vec4;
typedef struct Glyph Glyph;
typedef struct Vector Vector;
typedef struct Base Base;
typedef struct Button Button;
typedef struct Key Key;
typedef struct Base Base;
typedef struct Rect Rect;
typedef struct Shape Shape;
typedef struct Points Points;
typedef struct Line Line;
typedef struct Texture Texture;
typedef struct Font Font;
typedef struct Audio Audio;
typedef struct Image Image;
typedef struct Circle Circle;
typedef struct Text Text;
typedef struct Sound Sound;
typedef struct Screen Screen;
typedef struct Spec Spec;
typedef struct Filter Filter;
typedef struct Program Program;

struct Vec2 {
    double x;
    double y;
};

struct Vec3 {
    double x;
    double y;
    double z;
};

struct Vec4 {
    double x;
    double y;
    double z;
    double w;
};

struct Base {
    PyObject_HEAD
    Vec4 color;
    Vec2 pos;
    Vec2 scale;
    Vec2 anchor;
    double angle;
};

struct Rect {
    Base base;
    Vec2 size;
};

struct Shape {
    Base base;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    Vec2 *data;
    size_t len;
    size_t indices;
};

struct Line {
    Shape base;
    bool loop;
    double miter;
    double width;
};

struct Texture {
    Texture *next;
    char *name;
    int width;
    int height;
    GLuint src;
};

struct Glyph {
    uint32_t code;
    uint32_t jump;
    float adv;
    float x0;
    float y0;
    float x1;
    float y1;
    float s0;
    float t0;
    float s1;
    float t1;
};

struct Font {
    Font *next;
    uint8_t id;
    GLuint src;
    Glyph *chars;
    uint32_t len;
};

struct Audio {
    Audio *next;
    MIX_Audio *src;
    char *name;
};

struct Image {
    Rect base;
    Texture *src;
};

struct Text {
    Base base;
    wchar_t *content;
    size_t len;
    Font *src;
    GLuint vao;
    GLuint vbo;
    double size;
    double width;
};

struct Circle {
    Base base;
    double diameter;
};

struct Vector {
    PyObject_HEAD
    PyObject *parent;
    uint8_t size;
    int (*set)(PyObject *);
    double *var;
    char names[4];
};

struct Points {
    PyObject_HEAD
    Shape *parent;
    int (*update)(Shape *);
};

struct Sound {
    PyObject_HEAD
    MIX_Track *track;
    Audio *src;
    float *pcm;
    int samples;
    int channels;
};

struct Screen {
    Rect base;
    GLuint buffer;
    GLuint a;
    GLuint b;
};

struct Key {
    uint32_t id;
    const char *key;
    bool down;
    bool press;
    bool release;
    bool repeat;
};

struct Button {
    PyObject_HEAD
    Key *key;
};

struct Program {
    GLuint src;
    GLint obj;
    GLint color;
    GLint size;
};

struct Filter {
    GLuint src;
    GLint data;
};

struct Spec {
    PyType_Spec spec;
    PyTypeObject *type;
};

extern struct Window {
    SDL_Window *sdl;
    SDL_GLContext ctx;
    Vec2 size;
    Vec3 color;
    bool resize;
    char *title;
    double ratio;
} window;

extern struct Camera {
    Vec2 pos;
    Vec2 scale;
} camera;

extern struct Mouse {
    Vec2 pos;
    Vec2 move;
    Button *button;
    uint8_t len;
    bool press;
    bool release;
} mouse;

extern struct Keyboard {
    Button *key;
    Button *mod;
    PyObject map;
    uint16_t keys;
    uint8_t mods;
    bool press;
    bool release;
} keyboard;

extern struct Shader {
    Program *active;
    Screen *screen;
    Program plain;
    Program image;
    Program circle;
    Program text;
    Filter warp;
    GLuint ubo;
    GLuint vao;
    GLuint array;
    GLuint texture;
} shader;

extern struct Path {
    char *src;
    size_t size;
} path;

extern PyObject *error;
extern PyObject *program;
extern Texture *textures;
extern Font *fonts;
extern Audio *audio;
extern MIX_Mixer *mixer;

extern Spec vector_data;
extern Spec window_data;
extern Spec camera_data;
extern Spec mouse_data;
extern Spec key_data;
extern Spec sound_data;
extern Spec base_data;
extern Spec button_data;
extern Spec mod_data;
extern Spec rect_data;
extern Spec shape_data;
extern Spec points_data;
extern Spec line_data;
extern Spec image_data;
extern Spec circle_data;
extern Spec text_data;
extern Spec screen_data;

extern Vector *vector_new(PyObject *, double *, uint8_t, int (*)(PyObject *));
extern Points *points_new(Shape *, int (*)(Shape *));
extern PyObject *rect_intersect(PyObject *, Vec2 *);
extern PyObject *screen_bind(Base *, PyObject *, void (*)(Base *));

extern Vec2 shape_y(Shape *);
extern Vec2 shape_x(Shape *);
extern Vec2 *shape_points(Shape *);
extern Vec2 circle_pos(Circle *);

extern void base_trans(Base *, Vec2 *, Vec2 *, size_t);
extern void base_matrix(Base *, Program *, double, double);
extern void base_rect(Base *, Vec2 *, double, double);
extern double base_radius(Base *, double);
extern double rect_y(Base *, double, double, char);
extern double rect_x(Base *, double, double, char);

extern bool collide_poly_point(Vec2 *, size_t, Vec2);
extern bool collide_poly_circle(Vec2 *, size_t, Circle *);
extern bool collide_poly_poly(Vec2 *, size_t, Vec2 *, size_t);
extern bool collide_circle_point(Vec2, double, Vec2);
extern int collide_line_point(Line *, Vec2, double);
extern int collide_line_poly(Line *, Vec2 *, size_t);
extern int collide_line_line(Line *, Line *);

extern int base_top(Base *, PyObject *, double);
extern int base_right(Base *, PyObject *, double);
extern int base_bottom(Base *, PyObject *, double);
extern int base_left(Base *, PyObject *, double);
extern int button_compare(const char *, Button *);
extern int vector_set(PyObject *, double *, uint8_t);
extern int points_set(PyObject *, Shape *);

static inline Vec2 norm(double x, double y) {
    const double len = hypot(x, y);
    Vec2 value = {len ? x / len : 0, len ? y / len : 0};

    return value;
}

static inline void use(Program *program) {
    if (program != shader.active) {
        glUseProgram((shader.active = program) -> src);
        glUniform2f(program -> size, shader.screen ? shader.screen -> base.size.x : window.size.x, shader.screen ? -shader.screen -> base.size.y : window.size.y);
    }
}

static inline void array(GLuint vao) {
    if (vao != shader.array)
        glBindVertexArray(shader.array = vao);
}

static inline void texture(GLuint src) {
    if (src != shader.texture)
        glBindTexture(GL_TEXTURE_2D, shader.texture = src);
}

static inline void unbind(void) {
    if (shader.screen) {
        shader.screen = NULL;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window.size.x * window.ratio, window.size.y * window.ratio);

        if (shader.active)
            glUniform2f(shader.active -> size, window.size.x, window.size.y);
    }
}