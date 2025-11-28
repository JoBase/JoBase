#define DEL(e, n) if(!e)return PyErr_SetString(PyExc_AttributeError,"Cannot delete the '"n"' attribute"),-1;
#define REM(t, l, e) if (l==e)l=l->next;else for(t*i=l;i;i=i->next)if(i->next==e){i->next=i->next->next;break;}
#define INIT(e) if(e)return-1;
#define ERR(e) ((e)==-1&&PyErr_Occurred())
#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)
#define LEN(e) sizeof e/sizeof*e
#define _USE_MATH_DEFINES

#ifdef __EMSCRIPTEN__
#define VERSION "300 es"
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#define VERSION "330 core"
#include "glad/glad.h"
#endif

#include <Python.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <tesselator.h>
#include <stb_image.h>

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
typedef struct Image Image;
typedef struct Circle Circle;
typedef struct Text Text;
typedef struct Sound Sound;

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
};

extern struct Window {
    SDL_Window *sdl;
    SDL_GLContext ctx;
    Vec2 size;
    Vec3 color;
    bool resize;
    char *title;
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
    struct Program plain;
    struct Program image;
    struct Program circle;
    struct Program text;
    GLuint ubo;
    GLuint vao;
} shader;

extern struct Path {
    char *src;
    size_t size;
} path;

extern PyObject *error;
extern PyObject *program;
extern Texture *textures;
extern Font *fonts;

extern PyTypeObject VectorType;
extern PyTypeObject WindowType;
extern PyTypeObject KeyType;
extern PyTypeObject BaseType;
extern PyTypeObject ButtonType;
extern PyTypeObject ModType;
extern PyTypeObject RectType;
extern PyTypeObject CameraType;
extern PyTypeObject MouseType;
extern PyTypeObject ShapeType;
extern PyTypeObject PointsType;
extern PyTypeObject LineType;
extern PyTypeObject ImageType;
extern PyTypeObject CircleType;
extern PyTypeObject TextType;
extern PyTypeObject SoundType;

extern Vector *vector_new(PyObject *, double *, uint8_t, int (*)(PyObject *));
extern Points *points_new(Shape *, int (*)(Shape *));

extern void base_matrix(Base *, GLint, GLint, double, double);
extern int button_compare(const char *, Button *);
extern int vector_set(PyObject *, double *, uint8_t);
extern int points_set(PyObject *, Shape *);