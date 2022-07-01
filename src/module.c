#define PY_SSIZE_T_CLEAN
#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES

#define CHECK(i) if (!i) {return PyErr_SetString(PyExc_AttributeError, "can't delete attribute"), -1;}
#define ARRAY(i) return errorFormat(PyExc_TypeError, "must be sequence, not %s", Py_TYPE(i) -> tp_name), -1;
#define ITER(t, i) for (t *this = i; this; this = this -> next)
#define NEW(t, i) t *e = malloc(sizeof(t)); e -> next = i; i = e;
#define PARSE(e) wchar_t item; for (uint i = 0; (item = e[i]); i ++)
#define AVERAGE(i) (i[0] + i[1]) / 2
#define GET(t, i) t -> get(t -> parent)[i]
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define START ready = 0; glfwPollEvents();
#define END glfwWaitEventsTimeout(.1); ready = 1;

#define EXPAND(i) #i
#define STR(i) EXPAND(i)

#define SHAPE 0
#define IMAGE 1
#define TEXT 2
#define ADD 3
#define SUBTRACT 4
#define MULTIPLY 5
#define DIVIDE 6

#define STATIC CP_BODY_TYPE_STATIC
#define DYNAMIC CP_BODY_TYPE_DYNAMIC

#include <Python.h>
#include <structmember.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image/stb_image.h>
#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_unsafe.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef _WIN32
PyMODINIT_FUNC PyInit___init__;
#endif

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef double *vec;
typedef double vec2[2];
typedef double vec3[3];
typedef double vec4[4];
typedef float mat[16];
typedef uint uint2[2];
typedef int int2[2];
typedef vec2 poly[];
typedef vec2 tri[3];
typedef vec (*method)(PyObject *);

typedef struct Font {
    struct Font *next;
    FT_Face face;
    char *name;
} Font;

typedef struct Texture {
    struct Texture *next;
    uint source;
    uint2 size;
    char *name;
} Texture;

typedef struct Set {
    const char *key;
    uchar hold;
    uchar press;
    uchar release;
    uchar repeat;
} Set;

typedef struct Var {
    const char *name;
    setter set;
} Var;

typedef struct Char {
    uchar loaded;
    int fontSize;
    int advance;
    int2 size;
    int2 pos;
    uint source;
} Char;

typedef struct Vector {
    PyObject_HEAD
    PyObject *parent;
    method get;
    uchar size;
    Var data[4];
} Vector;

typedef struct Button {
    PyObject_HEAD
    Set *state;
} Button;

typedef struct Cursor {
    PyObject_HEAD
    Set buttons[GLFW_MOUSE_BUTTON_LAST + 1];
    uchar move;
    uchar enter;
    uchar leave;
    uchar press;
    uchar release;
} Cursor;

typedef struct Key {
    PyObject_HEAD
    Set keys[GLFW_KEY_LAST + 1];
    uchar press;
    uchar release;
    uchar repeat;
} Key;

typedef struct Camera {
    PyObject_HEAD
    vec2 pos;
} Camera;

typedef struct Window {
    PyObject_HEAD
    GLFWwindow *glfw;
    char *caption;
    vec3 color;
    uchar resize;
} Window;

typedef struct Base {
    PyObject_HEAD
    vec2 pos;
    vec2 scale;
    vec2 anchor;
    vec4 color;
    vec2 velocity;
    double angularVelocity;
    double angle;
    cpShape *shape;
    cpBody *body;
    int type;
    double mass;
    double elasticity;
    double friction;
    uchar rotate;
    cpFloat (*getMoment)(struct Base *);
    void (*newShape)(struct Base *);
    void (*setBase)(struct Base *);
    double (*getTop)(struct Base *);
    double (*getBottom)(struct Base *);
    double (*getLeft)(struct Base *);
    double (*getRight)(struct Base *);
} Base;

typedef struct Rectangle {
    Base base;
    vec2 size;
} Rectangle;

typedef struct Image {
    Rectangle rect;
    Texture *texture;
} Image;

typedef struct Text {
    Rectangle rect;
    wchar_t *content;
    Char *chars;
    Font *font;
    int2 base;
    int descender;
    double fontSize;
} Text;

typedef struct Circle {
    Base base;
    double radius;
    uint vao;
    uint vbo;
} Circle;

typedef struct Shape {
    Base base;
    vec2 *points;
    uint *indices;
    uint vertex;
    uint index;
    uint vao;
    uint vbo;
    uint ibo;
} Shape;

typedef struct Physics {
    PyObject_HEAD
    cpSpace *space;
    Base **data;
    uint length;
} Physics;

static Texture *textures;
static Font *fonts;
static Cursor *cursor;
static Key *key;
static Camera *camera;
static Window *window;

static PyTypeObject VectorType;
static PyTypeObject ButtonType;
static PyTypeObject BaseType;
static PyTypeObject CursorType;
static PyTypeObject RectangleType;
static PyTypeObject CircleType;
static PyTypeObject ShapeType;

static char *path;
static uchar ready;
static uint length;
static uint program;
static uint mesh;
static FT_Library library;
static PyObject *loop;

static const char *constructFilepath(const char *file) {
    return path[length] = 0, strcat(path, file);
}

static void setTextureParameters() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void errorFormat(PyObject *error, const char *format, ...) {
    va_list args;
    va_start(args, format);

    int size = vsnprintf(NULL, 0, format, args);
    char *buffer = malloc(size + 1);
    va_end(args);

    va_start(args, format);
    vsprintf(buffer, format, args);
    PyErr_SetString(error, buffer);

    va_end(args);
    free(buffer);
}

static vec getWindowSize() {
    static vec2 size;
    int width, height;

    glfwGetWindowSize(window -> glfw, &width, &height);
    size[0] = width;
    size[1] = height;

    return size;
}

static vec getCursorPos() {
    static vec2 pos;
    glfwGetCursorPos(window -> glfw, &pos[0], &pos[1]);

    vec size = getWindowSize();
    pos[0] -= size[0] / 2;
    pos[1] = size[1] / 2 - pos[1];

    return pos;
}

static int getOtherPos(PyObject *other, vec2 pos) {
    if (Py_TYPE(other) == &CursorType) {
        vec value = getCursorPos();

        pos[0] = value[0];
        pos[1] = value[1];
    }

    else if (PyObject_IsInstance(other, (PyObject *) &BaseType)) {
        pos[0] = ((Base *) other) -> pos[0];
        pos[1] = ((Base *) other) -> pos[1];
    }

    else if (PySequence_Check(other)) {
        PyObject *sequence = PySequence_Fast(other, NULL);

        if (PySequence_Fast_GET_SIZE(sequence) < 2) {
            PyErr_SetString(PyExc_ValueError, "sequence must contain 2 values");
            Py_DECREF(sequence);
            return -1;
        }

        for (uchar i = 0; i < 2; i ++) {
            pos[i] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(sequence, i));

            if (pos[i] == -1 && PyErr_Occurred()) {
                Py_DECREF(sequence);
                return -1;
            }
        }

        Py_DECREF(sequence);
    }

    else {
        errorFormat(PyExc_TypeError, "must be Base, cursor or sequence not %s", Py_TYPE(other) -> tp_name);
        return -1;
    }

    return 0;
}

static int moveToward(vec2 this, PyObject *args) {
    PyObject *other;
    double speed = 1;
    vec2 pos;

    if (!PyArg_ParseTuple(args, "O|d", &other, &speed) || getOtherPos(other, pos))
        return -1;

    const double x = pos[0] - this[0];
    const double y = pos[1] - this[1];

    if (hypot(x, y) < speed) {
        this[0] += x;
        this[1] += y;
    }

    else {
        const double angle = atan2(y, x);
        this[0] += cos(angle) * speed;
        this[1] += sin(angle) * speed;
    }
    
    return 0;
}

static int moveSmooth(vec2 this, PyObject *args) {
    PyObject *other;
    double speed = .1;
    vec2 pos;

    if (!PyArg_ParseTuple(args, "O|d", &other, &speed) || getOtherPos(other, pos))
        return -1;

    this[0] += (pos[0] - this[0]) * speed;
    this[1] += (pos[1] - this[1]) * speed;
    
    return 0;
}

static uchar triangulatePositive(poly poly, uint size) {
    double result = 0;

    for (uint p = size - 1, q = 0; q < size; p = q ++)
        result += poly[p][0] * poly[q][1] - poly[q][0] * poly[p][1];

    return result / 2 > 0;
}

static uchar triangulateInside(vec2 a, vec2 b, vec2 c, vec2 p) {
    const double ab = (c[0] - b[0]) * (p[1] - b[1]) - (c[1] - b[1]) * (p[0] - b[0]);
    const double ca = (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0]);
    const double bc = (a[0] - c[0]) * (p[1] - c[1]) - (a[1] - c[1]) * (p[0] - c[0]);

    return ab >= 0 && ca >= 0 && bc >= 0;
}

static uchar triangulateSnip(poly poly, uint u, uint v, uint w, uint next, uint *verts) {
    vec a = poly[verts[u]];
    vec b = poly[verts[v]];
    vec c = poly[verts[w]];

    if (DBL_EPSILON > (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]))
        return 0;

    for (uint i = 0; i < next; i ++) {
        if (i == u || i == v || i == w) continue;
        if (triangulateInside(a, b, c, poly[verts[i]])) return 0;
    }

    return 1;
}

static int triangulatePoly(poly poly, uint size, uint **indices, uint *index) {
    uint *verts = malloc(size * 4);
    uint *data = *indices;
    uint count = size * 2;
    uint next = size;
    *index = 0;

    if (triangulatePositive(poly, size))
        for (uint i = 0; i < size; i ++)
            verts[i] = i;

    else for (uint i = 0; i < size; i ++)
        verts[i] = (size - 1) - i;

    for (uint v = next - 1; next > 2;) {
        if (count -- <= 0) {
            PyErr_SetString(PyExc_ValueError, "failed to understand shape - probably because the edges overlap");
            free(verts);
            return -1;
        }

        uint u = next > v ? v : 0;
        v = next > u + 1 ? u + 1 : 0;
        uint w = next > v + 1 ? v + 1 : 0;

        if (triangulateSnip(poly, u, v, w, next, verts)) {
            data = realloc(data, (*index + 1) * 12);
            data[*index] = verts[u];
            data[*index + 1] = verts[v];
            data[*index + 2] = verts[w];

            *index += 3;
            *indices = data;

            for (uint s = v, t = v + 1; t < next; s ++, t ++)
                verts[s] = verts[t];

            next --;
            count = next * 2;
        }
    }

    return free(verts), 0;
}

static uchar collideCircleCircle(vec2 p1, double r1, vec2 p2, double r2) {
    return hypot(p2[0] - p1[0], p2[1] - p1[1]) < r1 + r2;
}

static uchar collideCirclePoint(vec2 pos, double radius, vec2 point) {
    return hypot(point[0] - pos[0], point[1] - pos[1]) < radius;
}

static uchar collideLinePoint(vec2 p1, vec2 p2, vec2 point) {
    const double d1 = hypot(point[0] - p1[0], point[1] - p1[1]);
    const double d2 = hypot(point[0] - p2[0], point[1] - p2[1]);
    const double length = hypot(p1[0] - p2[0], p1[1] - p2[1]);

    return d1 + d2 >= length - .1 && d1 + d2 <= length + .1;
}

static uchar collideLineLine(vec2 p1, vec2 p2, vec2 p3, vec2 p4) {
    const double value = (p4[1] - p3[1]) * (p2[0] - p1[0]) - (p4[0] - p3[0]) * (p2[1] - p1[1]);
    const double u1 = ((p4[0] - p3[0]) * (p1[1] - p3[1]) - (p4[1] - p3[1]) * (p1[0] - p3[0])) / value;
    const double u2 = ((p2[0] - p1[0]) * (p1[1] - p3[1]) - (p2[1] - p1[1]) * (p1[0] - p3[0])) / value;

    return u1 >= 0 && u1 <= 1 && u2 >= 0 && u2 <= 1;
}

static uchar collideLineCircle(vec2 p1, vec2 p2, vec2 pos, double radius) {
    if (collideCirclePoint(pos, radius, p1) || collideCirclePoint(pos, radius, p2))
        return 1;

    const double length = hypot(p1[0] - p2[0], p1[1] - p2[1]);
    const double dot = ((pos[0] - p1[0]) * (p2[0] - p1[0]) + (pos[1] - p1[1]) * (p2[1] - p1[1])) / pow(length, 2);
    vec2 point = {p1[0] + dot * (p2[0] - p1[0]), p1[1] + dot * (p2[1] - p1[1])};

    if (!collideLinePoint(p1, p2, point))
        return 0;

    return hypot(point[0] - pos[0], point[1] - pos[1]) <= radius;
}

static uchar collidePolyLine(poly poly, uint size, vec2 p1, vec2 p2) {
    for (uint i = 0; i < size; i ++)
        if (collideLineLine(p1, p2, poly[i], poly[i + 1 == size ? 0 : i + 1]))
            return 1;

    return 0;
}

static uchar collidePolyPoint(poly poly, uint size, vec2 point) {
    uchar hit = 0;

    for (uint i = 0; i < size; i ++) {
        vec a = poly[i];
        vec b = poly[i + 1 == size ? 0 : i + 1];

        if ((point[0] < (b[0] - a[0]) * (point[1] - a[1]) / (b[1] - a[1]) + a[0]) &&
            ((a[1] > point[1] && b[1] < point[1]) ||
            (a[1] < point[1] && b[1] > point[1]))) hit = !hit;
    }

    return hit;
}

static uchar collidePolyPoly(poly p1, uint s1, poly p2, uint s2) {
    if (collidePolyPoint(p1, s1, p2[0]) || collidePolyPoint(p2, s2, p1[0]))
        return 1;

    for (uint i = 0; i < s1; i ++)
        if (collidePolyLine(p2, s2, p1[i], p1[i + 1 == s1 ? 0 : i + 1]))
            return 1;

    return 0;
}

static uchar collidePolyCircle(poly poly, uint size, vec2 pos, double radius) {
    for (uint i = 0; i < size; i ++)
        if (collideLineCircle(poly[i], poly[i + 1 == size ? 0 : i + 1], pos, radius))
            return 1;
    
    return 0;
}

static void rotPoly(poly poly, uint size, double angle, vec2 pos) {
    const double cosine = cos(angle * M_PI / 180);
    const double sine = sin(angle * M_PI / 180);

    for (uint i = 0; i < size; i ++) {
        const double x = poly[i][0];
        const double y = poly[i][1];

        poly[i][0] = x * cosine - y * sine + pos[0];
        poly[i][1] = x * sine + y * cosine + pos[1];
    }
}

static double getPolyLeft(poly poly, uint size) {
    double left = poly[0][0];

    for (uint i = 1; i < size; i ++)
        if (poly[i][0] < left)
            left = poly[i][0];

    return left;
}

static double getPolyTop(poly poly, uint size) {
    double top = poly[0][1];

    for (uint i = 1; i < size; i ++)
        if (poly[i][1] > top)
            top = poly[i][1];

    return top;
}

static double getPolyRight(poly poly, uint size) {
    double right = poly[0][0];

    for (uint i = 1; i < size; i ++)
        if (poly[i][0] > right)
            right = poly[i][0];

    return right;
}

static double getPolyBottom(poly poly, uint size) {
    double bottom = poly[0][1];

    for (uint i = 1; i < size; i ++)
        if (poly[i][1] < bottom)
            bottom = poly[i][1];

    return bottom;
}

static void getRectPoly(Rectangle *self, poly poly) {
    const double px = self -> size[0] * self -> base.scale[0] / 2;
    const double py = self -> size[1] * self -> base.scale[1] / 2;

    poly[0][0] = poly[3][0] = self -> base.anchor[0] - px;
    poly[0][1] = poly[1][1] = self -> base.anchor[1] + py;
    poly[1][0] = poly[2][0] = self -> base.anchor[0] + px;
    poly[2][1] = poly[3][1] = self -> base.anchor[1] - py;

    rotPoly(poly, 4, self -> base.angle, self -> base.pos);
}

static vec2 *getShapePoly(Shape *self) {
    vec2 *poly = malloc(self -> vertex * 16);

    for (uint i = 0; i < self -> vertex; i ++) {
        poly[i][0] = self -> points[i][0] + self -> base.anchor[0];
        poly[i][1] = self -> points[i][1] + self -> base.anchor[1];
    }

    rotPoly(poly, self -> vertex, self -> base.angle, self -> base.pos);
    return poly;
}

static vec getBaseCenter(Base *base) {
    const double angle = base -> angle * M_PI / 180;
    static vec2 pos;

    pos[0] = base -> pos[0] + base -> anchor[0] * cos(angle) - base -> anchor[1] * sin(angle);
    pos[1] = base -> pos[1] + base -> anchor[1] * cos(angle) + base -> anchor[0] * sin(angle);
    return pos;
}

static void setUniform(mat matrix, vec4 color) {
    glUniformMatrix4fv(glGetUniformLocation(program, "object"), 1, GL_FALSE, matrix);

    glUniform4f(
        glGetUniformLocation(program, "color"), (float) color[0],
        (float) color[1], (float) color[2], (float) color[3]);
}

static int mainLoop() {
    vec size = getWindowSize();

    const float sx = (float) size[0];
    const float sy = (float) size[1];
    const float px = (float) camera -> pos[0];
    const float py = (float) camera -> pos[1];

    mat matrix = {
        2 / sx, 0, 0, 0,
        0, 2 / sy, 0, 0,
        0, 0, -2, 0,
        -px * 2 / sx, -py * 2 / sy, -1, 1
    };
    
    glUniformMatrix4fv(glGetUniformLocation(program, "camera"), 1, GL_FALSE, matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    if (loop && !PyObject_CallObject(loop, NULL)) {
        Py_DECREF(loop);
        return -1;
    }

    window -> resize = 0;
    cursor -> move = 0;
    cursor -> enter = 0;
    cursor -> leave = 0;
    cursor -> press = 0;
    cursor -> release = 0;
    key -> press = 0;
    key -> release = 0;
    key -> repeat = 0;

    for (ushort i = 0; i <= GLFW_KEY_LAST; i ++) {
        key -> keys[i].press = 0;
        key -> keys[i].release = 0;
        key -> keys[i].repeat = 0;
    }

    for (ushort i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i ++) {
        cursor -> buttons[i].press = 0;
        cursor -> buttons[i].release = 0;
    }

    return glfwSwapBuffers(window -> glfw), 0;
}

static void windowRefreshCallback(GLFWwindow *Py_UNUSED(window)) {
    if (ready && !PyErr_Occurred()) mainLoop();
}

static void windowSizeCallback(GLFWwindow *Py_UNUSED(window), int Py_UNUSED(width), int Py_UNUSED(height)) {
    window -> resize = 1;
}

static void framebufferSizeCallback(GLFWwindow *Py_UNUSED(window), int width, int height) {
    glViewport(0, 0, width, height);
}

static void cursorPosCallback(GLFWwindow *Py_UNUSED(window), double Py_UNUSED(x), double Py_UNUSED(y)) {
    cursor -> move = 1;
}

static void cursorEnterCallback(GLFWwindow *Py_UNUSED(window), int entered) {
    entered ? (cursor -> enter = 1) : (cursor -> leave = 1);
}

static void mouseButtonCallback(GLFWwindow *Py_UNUSED(window), int button, int action, int Py_UNUSED(mods)) {
    if (action == GLFW_PRESS) {
        cursor -> press = 1;
        cursor -> buttons[button].press = 1;
        cursor -> buttons[button].hold = 1;
    }

    else if (action == GLFW_RELEASE) {
        cursor -> release = 1;
        cursor -> buttons[button].release = 1;
        cursor -> buttons[button].hold = 0;
    }
}

static void keyCallback(GLFWwindow *Py_UNUSED(window), int type, int Py_UNUSED(scancode), int action, int Py_UNUSED(mods)) {
    if (action == GLFW_PRESS) {
        key -> press = 1;
        key -> keys[type].press = 1;
        key -> keys[type].hold = 1;
    }

    else if (action == GLFW_RELEASE) {
        key -> release = 1;
        key -> keys[type].release = 1;
        key -> keys[type].hold = 0;
    }

    else if (action == GLFW_REPEAT) {
        key -> repeat = 1;
        key -> keys[type].repeat = 1;
    }
}

static PyObject *Object_collidesWith(PyObject *self, PyObject *other) {
    #define LONG(i) PyBool_FromLong(i);
    #define RECT(r) PyObject_IsInstance(r, (PyObject *) &RectangleType)
    #define OBJECT(o) return errorFormat(PyExc_TypeError, "must be Base or cursor, not %s", Py_TYPE(o) -> tp_name), NULL;

    if (RECT(self)) {
        vec2 rect[4];
        getRectPoly((Rectangle *) self, rect);

        if (RECT(other)) {
            vec2 poly[4];

            getRectPoly((Rectangle *) other, poly);
            return LONG(collidePolyPoly(rect, 4, poly, 4));
        }

        if (Py_TYPE(other) == &CircleType) {
            Circle *circle = (Circle *) other;

            return LONG(collidePolyCircle(
                rect, 4, getBaseCenter((Base *) circle),
                circle -> radius * AVERAGE(circle -> base.scale)));
        }

        if (Py_TYPE(other) == &ShapeType) {
            Shape *shape = (Shape *) other;
            vec2 *poly = getShapePoly(shape);

            PyObject *value = LONG(collidePolyPoly(rect, 4, poly, shape -> vertex));
            return free(poly), value;
        }

        if (other == (PyObject *) cursor) return LONG(collidePolyPoint(
            rect, 4, getCursorPos()));

        OBJECT(other)
    }

    if (Py_TYPE(self) == &CircleType) {
        Circle *circle = (Circle *) self;
        const double size = circle -> radius * AVERAGE(circle -> base.scale);
        vec pos = getBaseCenter((Base *) circle);

        if (RECT(other)) {
            vec2 rect[4];

            getRectPoly((Rectangle *) other, rect);
            return LONG(collidePolyCircle(rect, 4, pos, size));
        }

        if (Py_TYPE(other) == &CircleType) {
            Circle *other = (Circle *) other;

            return LONG(collideCircleCircle(
                pos, size, getBaseCenter((Base *) other),
                other -> radius * AVERAGE(other -> base.scale)));
        }

        if (Py_TYPE(other) == &ShapeType) {
            Shape *shape = (Shape *) other;
            vec2 *poly = getShapePoly(shape);

            PyObject *value = LONG(collidePolyCircle(poly, shape -> vertex, pos, size));
            return free(poly), value;
        }

        if (other == (PyObject *) cursor) return LONG(collideCirclePoint(
            pos, size, getCursorPos()));

        OBJECT(other)
    }

    if (Py_TYPE(self) == &ShapeType) {
        Shape *shape = (Shape *) self;
        vec2 *poly = getShapePoly(shape);

        if (RECT(other)) {
            vec2 rect[4];
            getRectPoly((Rectangle *) other, rect);

            PyObject *value = LONG(collidePolyPoly(poly, shape -> vertex, rect, 4));
            return free(poly), value;
        }

        if (Py_TYPE(other) == &CircleType) {
            Circle *circle = (Circle *) other;

            PyObject *value = LONG(collidePolyCircle(
                poly, shape -> vertex, getBaseCenter((Base *) circle),
                circle -> radius * AVERAGE(circle -> base.scale)));

            return free(poly), value;
        }

        if (Py_TYPE(other) == &ShapeType) {
            Shape *other = (Shape *) other;
            vec2 *mesh = getShapePoly(other);

            PyObject *value = LONG(collidePolyPoly(poly, shape -> vertex, mesh, other -> vertex));
            return free(poly), free(mesh), value;
        }

        if (other == (PyObject *) cursor) {
            PyObject *value = LONG(collidePolyPoint(poly, shape -> vertex, getCursorPos()));
            return free(poly), value;
        }

        free(poly);
        OBJECT(other)
    }

    if (self == (PyObject *) cursor) {
        if (RECT(other)) {
            vec2 rect[4];

            getRectPoly((Rectangle *) other, rect);
            return LONG(collidePolyPoint(rect, 4, getCursorPos()));
        }

        if (Py_TYPE(other) == &CircleType) {
            Circle *circle = (Circle *) other;

            return LONG(collideCirclePoint(
                getBaseCenter((Base *) circle),
                circle -> radius * AVERAGE(circle -> base.scale), getCursorPos()));
        }

        if (Py_TYPE(other) == &ShapeType) {
            Shape *shape = (Shape *) other;
            vec2 *poly = getShapePoly(shape);

            PyObject *value = LONG(collidePolyPoint(poly, shape -> vertex, getCursorPos()));
            return free(poly), value;
        }

        if (other == (PyObject *) cursor)
            Py_RETURN_TRUE;

        OBJECT(other)
    }

    OBJECT(self)
    #undef LONG
    #undef RECT
    #undef OBJECT
}

static double vectorCheck(double a, double b, uchar type) {
    switch (type) {
        case ADD: return a + b;
        case SUBTRACT: return a - b;
        case MULTIPLY: return a * b;
        default: return a / b;
    }
}

static PyObject *vectorPrint(Vector *self, uchar a, uchar b) {
    char *buffer = malloc(14 * self -> size + 1);
    uchar count = 1;
    buffer[0] = a;

    for (uchar i = 0; i < self -> size; i ++) {
        if (i) {
            buffer[count ++] = 44;
            buffer[count ++] = 32;
        }

        int length = sprintf(&buffer[count], "%g", GET(self, i));
        count += length;
    }

    buffer[count] = b;
    PyObject *result = PyUnicode_FromString(buffer);

    return free(buffer), result;
}

static int vectorSet(PyObject *value, vec vector, uchar size) {
    CHECK(value)

    if (Py_TYPE(value) == &VectorType) {
        Vector *object = (Vector *) value;

        for (uchar i = 0; i < MIN(size, object -> size); i ++)
            vector[i] = GET(object, i);
    }

    else if (PyNumber_Check(value)) {
        const double number = PyFloat_AsDouble(value);
        if (number == -1 && PyErr_Occurred()) return -1;

        for (uchar i = 0; i < size; i ++)
            vector[i] = number;
    }

    else if (PySequence_Check(value)) {
        PyObject *sequence = PySequence_Fast(value, NULL);
        Py_ssize_t length = PySequence_Fast_GET_SIZE(sequence);

        for (uchar i = 0; i < MIN(size, length); i ++) {
            vector[i] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(sequence, i));

            if (vector[i] == -1 && PyErr_Occurred()) {
                Py_DECREF(sequence);
                return -1;
            }
        }

        Py_DECREF(sequence);
    }

    else ARRAY(value)
    return 0;
}

static Vector *vectorNew(PyObject *parent, method get, uchar size) {
    Vector *array = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!array) return NULL;

    array -> parent = parent;
    array -> get = get;
    array -> size = size;

    return Py_INCREF(parent), array;
}

static PyObject *vectorNumber(Vector *self, PyObject *other, uchar type) {
    if (PyNumber_Check(other)) {
        PyObject *result = PyTuple_New(self -> size);

        const double value = PyFloat_AsDouble(other);
        if (value == -1 && PyErr_Occurred()) return NULL;

        for (uchar i = 0; i < self -> size; i ++) {
            PyObject *current = PyFloat_FromDouble(vectorCheck(GET(self, i), value, type));
            if (!current) return NULL;

            PyTuple_SET_ITEM(result, i, current);
        }

        return result;
    }

    if (Py_TYPE(other) == &VectorType) {
        Vector *object = (Vector *) other;
        PyObject *result = PyTuple_New(MAX(self -> size, object -> size));

        for (uchar i = 0; i < MAX(self -> size, object -> size); i ++) {
            PyObject *current = PyFloat_FromDouble(
                i >= self -> size ? GET(object, i) :
                i >= object -> size ? GET(self, i) :
                vectorCheck(GET(self, i), GET(object, i), type));

            if (!current) return NULL;
            PyTuple_SET_ITEM(result, i, current);
        }

        return result;
    }

    errorFormat(PyExc_TypeError, "must be Vector or number, not %s", Py_TYPE(other) -> tp_name);
    return NULL;
}

static Py_ssize_t Vector_len(Vector *self) {
    return self -> size;
}

static PyObject *Vector_item(Vector *self, Py_ssize_t index) {
    return index >= self -> size ? PyErr_SetString(
        PyExc_IndexError, "index out of range"), NULL : PyFloat_FromDouble(GET(self, index));
}

static PySequenceMethods VectorSequenceMethods = {
    .sq_length = (lenfunc) Vector_len,
    .sq_item = (ssizeargfunc) Vector_item
};

static PyObject *Vector_add(Vector *self, PyObject *other) {
    return vectorNumber(self, other, ADD);
}

static PyObject *Vector_subtract(Vector *self, PyObject *other) {
    return vectorNumber(self, other, SUBTRACT);
}

static PyObject *Vector_multiply(Vector *self, PyObject *other) {
    return vectorNumber(self, other, MULTIPLY);
}

static PyObject *Vector_trueDivide(Vector *self, PyObject *other) {
    return vectorNumber(self, other, DIVIDE);
}

static PyNumberMethods VectorNumberMethods = {
    .nb_add = (binaryfunc) Vector_add,
    .nb_subtract = (binaryfunc) Vector_subtract,
    .nb_multiply = (binaryfunc) Vector_multiply,
    .nb_true_divide = (binaryfunc) Vector_trueDivide
};

static void Vector_dealloc(Vector *self) {
    Py_DECREF(self -> parent);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyObject *Vector_richcompare(Vector *self, PyObject *other, int op) {
    #define COMPARE if ((op == Py_LT && a < b) || (op == Py_GT && a > b) || ( \
        op == Py_LE && a <= b) || (op == Py_GE && a >= b)) Py_RETURN_TRUE; Py_RETURN_FALSE;

    if (PyNumber_Check(other)) {
        if (op == Py_EQ || op == Py_NE)
            Py_RETURN_FALSE;

        double a = 1;
        const double b = PyFloat_AsDouble(other);

        if (b == -1 && PyErr_Occurred()) return NULL;
        for (uchar i = 0; i < self -> size; i ++) a *= GET(self, i);

        COMPARE
    }

    if (Py_TYPE(other) == &VectorType) {
        Vector *object = (Vector *) other;

        if (op == Py_EQ || op == Py_NE) {
            uchar ne = 0;

            for (uchar i = 0; i < MIN(self -> size, object -> size); i ++)
                if (GET(self, i) != GET(object, i)) {
                    if (op == Py_EQ) Py_RETURN_FALSE;
                    ne = 1;
                }

            if (op == Py_EQ || ne)
                Py_RETURN_TRUE;

            Py_RETURN_FALSE;
        }

        double a = 1, b = 1;
        for (uchar i = 0; i < self -> size; i ++)  a *= GET(self, i);
        for (uchar i = 0; i < object -> size; i ++) b *= GET(object, i);

        COMPARE
    }

    errorFormat(PyExc_TypeError, "must be Vector or number, not %s", Py_TYPE(other) -> tp_name);
    return NULL;
    #undef COMPARE
}

static PyObject *Vector_str(Vector *self) {
    return vectorPrint(self, 40, 41);
}

static PyObject *Vector_repr(Vector *self) {
    return vectorPrint(self, 91, 93);
}

static PyObject *Vector_getattro(Vector *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return NULL;

    for (uchar i = 0; i < self -> size; i ++)
        if (!strcmp(name, self -> data[i].name))
            return PyFloat_FromDouble(GET(self, i));

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static int Vector_setattro(Vector *self, PyObject *attr, PyObject *value) {
    CHECK(value)

    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return -1;
    
    for (uchar i = 0; i < 4; i ++)
        if (!strcmp(name, self -> data[i].name))
            return self -> data[i].set(self -> parent, value, NULL);

    return PyObject_GenericSetAttr((PyObject *) self, attr, value);
}

static PyTypeObject VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Vector",
    .tp_doc = "a position, color or set of values",
    .tp_basicsize = sizeof(Vector),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Vector_dealloc,
    .tp_richcompare = (richcmpfunc) Vector_richcompare,
    .tp_str = (reprfunc) Vector_str,
    .tp_repr = (reprfunc) Vector_repr,
    .tp_getattro = (getattrofunc) Vector_getattro,
    .tp_setattro = (setattrofunc) Vector_setattro,
    .tp_as_number = &VectorNumberMethods,
    .tp_as_sequence = &VectorSequenceMethods
};

static Button *buttonNew(Set *state) {
    Button *button = (Button *) PyObject_CallObject((PyObject *) &ButtonType, NULL);
    if (!button) return NULL;

    button -> state = state;
    return button;
}

static int Button_bool(Button *self) {
    return self -> state -> hold || self -> state -> release;
}

static PyNumberMethods ButtonNumberMethods = {
    .nb_bool = (inquiry) Button_bool
};

static PyObject *Button_getPress(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> press);
}

static PyObject *Button_getRelease(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> release);
}

static PyObject *Button_getRepeat(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> repeat);
}

static PyObject *Button_getHold(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> hold);
}

static PyGetSetDef ButtonGetSetters[] = {
    {"press", (getter) Button_getPress, NULL, "the button is pressed", NULL},
    {"release", (getter) Button_getRelease, NULL, "the button is released", NULL},
    {"repeat", (getter) Button_getRepeat, NULL, "the button repeat is triggered", NULL},
    {"hold", (getter) Button_getHold, NULL, "the button is held down", NULL},
    {NULL}
};

static PyObject *Button_str(Button *self) {
    return PyUnicode_FromString(
        self -> state -> hold || self -> state -> release ? "True" : "False");
}

static PyTypeObject ButtonType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Button",
    .tp_doc = "the state of a keyboard or mouse button",
    .tp_basicsize = sizeof(Button),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_str = (reprfunc) Button_str,
    .tp_getset = ButtonGetSetters,
    .tp_as_number = &ButtonNumberMethods
};

static PyObject *Cursor_getX(Cursor *Py_UNUSED(self), void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(getCursorPos()[0]);
}

static int Cursor_setX(Cursor *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const double x = PyFloat_AsDouble(value);
    if (x == -1 && PyErr_Occurred())return -1;

    START
    glfwSetCursorPos(window -> glfw, x + getWindowSize()[0] / 2, getCursorPos()[1]);
    END return 0;
}

static PyObject *Cursor_getY(Cursor *Py_UNUSED(self), void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(getCursorPos()[1]);
}

static int Cursor_setY(Cursor *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const double y = PyFloat_AsDouble(value);
    if (y == -1 && PyErr_Occurred()) return -1;

    START
    glfwSetCursorPos(window -> glfw, getCursorPos()[0], getWindowSize()[1] / 2 - y);
    END return 0;
}

static PyObject *Cursor_getPos(Cursor *self, void *Py_UNUSED(closure)) {
    Vector *pos = vectorNew((PyObject *) self, (method) getCursorPos, 2);
    pos -> data[0].name = "x";
    pos -> data[1].name = "y";

    return (PyObject *) pos;
}

static int Cursor_setPos(Cursor *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    vec pos = getCursorPos();
    vec size = getWindowSize();

    if (vectorSet(value, pos, 2))
        return -1;

    START
    glfwSetCursorPos(window -> glfw, pos[0] + size[0] / 2, size[1] / 2 - pos[1]);
    END return 0;
}

static PyObject *Cursor_getMove(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> move);
}

static PyObject *Cursor_getEnter(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> enter);
}

static PyObject *Cursor_getLeave(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> leave);
}

static PyObject *Cursor_getPress(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> press);
}

static PyObject *Cursor_getRelease(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> release);
}

static PyObject *Cursor_getHold(Cursor *self, void *Py_UNUSED(closure)) {
    for (ushort i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i ++)
        if (self -> buttons[i].hold)
            Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyGetSetDef CursorGetSetters[] = {
    {"x", (getter) Cursor_getX, (setter) Cursor_setX, "x position of the cursor", NULL},
    {"y", (getter) Cursor_getY, (setter) Cursor_setY, "y position of the cursor", NULL},
    {"position", (getter) Cursor_getPos, (setter) Cursor_setPos, "position of the cursor", NULL},
    {"pos", (getter) Cursor_getPos, (setter) Cursor_setPos, "position of the cursor", NULL},
    {"move", (getter) Cursor_getMove, NULL, "the cursor has moved", NULL},
    {"enter", (getter) Cursor_getEnter, NULL, "the cursor has entered the window", NULL},
    {"leave", (getter) Cursor_getLeave, NULL, "the cursor has left the window", NULL},
    {"press", (getter) Cursor_getPress, NULL, "a mouse button is pressed", NULL},
    {"release", (getter) Cursor_getRelease, NULL, "a mouse button is released", NULL},
    {"hold", (getter) Cursor_getHold, NULL, "a mouse button is held down", NULL},
    {NULL}
};

static PyMethodDef CursorMethods[] = {
    {"collides_with", (PyCFunction) Object_collidesWith, METH_O, "check if the cursor collides with another object"},
    {NULL}
};

static PyObject *Cursor_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Cursor *self = cursor = (Cursor *) type -> tp_alloc(type, 0);
    Set *e = self -> buttons;

    e[GLFW_MOUSE_BUTTON_LEFT].key = "left";
    e[GLFW_MOUSE_BUTTON_RIGHT].key = "right";
    e[GLFW_MOUSE_BUTTON_MIDDLE].key = "middle";
    e[GLFW_MOUSE_BUTTON_4].key = "_4";
    e[GLFW_MOUSE_BUTTON_5].key = "_5";
    e[GLFW_MOUSE_BUTTON_6].key = "_6";
    e[GLFW_MOUSE_BUTTON_7].key = "_7";
    e[GLFW_MOUSE_BUTTON_8].key = "_8";

    Py_XINCREF(self);
    return (PyObject *) self;
}

static PyObject *Cursor_getattro(Cursor *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return NULL;
    
    for (ushort i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i ++)
        if (self -> buttons[i].key && !strcmp(self -> buttons[i].key, name))
            return (PyObject *) buttonNew(&self -> buttons[i]);

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static PyTypeObject CursorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Cursor",
    .tp_doc = "the cursor or mouse input handler",
    .tp_basicsize = sizeof(Cursor),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Cursor_new,
    .tp_getattro = (getattrofunc) Cursor_getattro,
    .tp_getset = CursorGetSetters,
    .tp_methods = CursorMethods
};

static PyObject *Key_getPress(Key *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> press);
}

static PyObject *Key_getRelease(Key *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> release);
}

static PyObject *Key_getRepeat(Key *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> repeat);
}

static PyObject *Key_getHold(Key *self, void *Py_UNUSED(closure)) {
    for (ushort i = 0; i <= GLFW_KEY_LAST; i ++)
        if (self -> keys[i].hold)
            Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyGetSetDef KeyGetSetters[] = {
    {"press", (getter) Key_getPress, NULL, "a key is pressed", NULL},
    {"release", (getter) Key_getRelease, NULL, "a key is released", NULL},
    {"repeat", (getter) Key_getRepeat, NULL, "triggered when a key is held down", NULL},
    {"hold", (getter) Key_getHold, NULL, "a key is held down", NULL},
    {NULL}
};

static PyObject *Key_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Key *self = key = (Key *) type -> tp_alloc(type, 0);
    Set *e = self -> keys;

    e[GLFW_KEY_SPACE].key = "space";
    e[GLFW_KEY_APOSTROPHE].key = "apostrophe";
    e[GLFW_KEY_COMMA].key = "comma";
    e[GLFW_KEY_MINUS].key = "minus";
    e[GLFW_KEY_PERIOD].key = "period";
    e[GLFW_KEY_SLASH].key = "slash";
    e[GLFW_KEY_0].key = "_0";
    e[GLFW_KEY_1].key = "_1";
    e[GLFW_KEY_2].key = "_2";
    e[GLFW_KEY_3].key = "_3";
    e[GLFW_KEY_4].key = "_4";
    e[GLFW_KEY_5].key = "_5";
    e[GLFW_KEY_6].key = "_6";
    e[GLFW_KEY_7].key = "_7";
    e[GLFW_KEY_8].key = "_8";
    e[GLFW_KEY_9].key = "_9";
    e[GLFW_KEY_SEMICOLON].key = "semicolon";
    e[GLFW_KEY_EQUAL].key = "equal";
    e[GLFW_KEY_A].key = "a";
    e[GLFW_KEY_B].key = "b";
    e[GLFW_KEY_C].key = "c";
    e[GLFW_KEY_D].key = "d";
    e[GLFW_KEY_E].key = "e";
    e[GLFW_KEY_F].key = "f";
    e[GLFW_KEY_G].key = "g";
    e[GLFW_KEY_H].key = "h";
    e[GLFW_KEY_I].key = "i";
    e[GLFW_KEY_J].key = "j";
    e[GLFW_KEY_K].key = "k";
    e[GLFW_KEY_L].key = "l";
    e[GLFW_KEY_M].key = "m";
    e[GLFW_KEY_N].key = "n";
    e[GLFW_KEY_O].key = "o";
    e[GLFW_KEY_P].key = "p";
    e[GLFW_KEY_Q].key = "q";
    e[GLFW_KEY_R].key = "r";
    e[GLFW_KEY_S].key = "s";
    e[GLFW_KEY_T].key = "t";
    e[GLFW_KEY_U].key = "u";
    e[GLFW_KEY_V].key = "v";
    e[GLFW_KEY_W].key = "w";
    e[GLFW_KEY_X].key = "x";
    e[GLFW_KEY_Y].key = "y";
    e[GLFW_KEY_Z].key = "z";
    e[GLFW_KEY_LEFT_BRACKET].key = "left_bracket";
    e[GLFW_KEY_BACKSLASH].key = "backslash";
    e[GLFW_KEY_RIGHT_BRACKET].key = "right_bracket";
    e[GLFW_KEY_GRAVE_ACCENT].key = "backquote";
    e[GLFW_KEY_ESCAPE].key = "escape";
    e[GLFW_KEY_ENTER].key = "enter";
    e[GLFW_KEY_TAB].key = "tab";
    e[GLFW_KEY_BACKSPACE].key = "backspace";
    e[GLFW_KEY_INSERT].key = "insert";
    e[GLFW_KEY_DELETE].key = "delete";
    e[GLFW_KEY_RIGHT].key = "right";
    e[GLFW_KEY_LEFT].key = "left";
    e[GLFW_KEY_DOWN].key = "down";
    e[GLFW_KEY_UP].key = "up";
    e[GLFW_KEY_PAGE_UP].key = "page_up";
    e[GLFW_KEY_PAGE_DOWN].key = "page_down";
    e[GLFW_KEY_HOME].key = "home";
    e[GLFW_KEY_END].key = "end";
    e[GLFW_KEY_CAPS_LOCK].key = "caps_lock";
    e[GLFW_KEY_SCROLL_LOCK].key = "scroll_lock";
    e[GLFW_KEY_NUM_LOCK].key = "num_lock";
    e[GLFW_KEY_PRINT_SCREEN].key = "print_screen";
    e[GLFW_KEY_PAUSE].key = "pause";
    e[GLFW_KEY_F1].key = "f1";
    e[GLFW_KEY_F2].key = "f2";
    e[GLFW_KEY_F3].key = "f3";
    e[GLFW_KEY_F4].key = "f4";
    e[GLFW_KEY_F5].key = "f5";
    e[GLFW_KEY_F6].key = "f6";
    e[GLFW_KEY_F7].key = "f7";
    e[GLFW_KEY_F8].key = "f8";
    e[GLFW_KEY_F9].key = "f9";
    e[GLFW_KEY_F10].key = "f10";
    e[GLFW_KEY_F11].key = "f11";
    e[GLFW_KEY_F12].key = "f12";
    e[GLFW_KEY_F13].key = "f13";
    e[GLFW_KEY_F14].key = "f14";
    e[GLFW_KEY_F15].key = "f15";
    e[GLFW_KEY_F16].key = "f16";
    e[GLFW_KEY_F17].key = "f17";
    e[GLFW_KEY_F18].key = "f18";
    e[GLFW_KEY_F19].key = "f19";
    e[GLFW_KEY_F20].key = "f20";
    e[GLFW_KEY_F21].key = "f21";
    e[GLFW_KEY_F22].key = "f22";
    e[GLFW_KEY_F23].key = "f23";
    e[GLFW_KEY_F24].key = "f24";
    e[GLFW_KEY_F25].key = "f25";
    e[GLFW_KEY_KP_0].key = "pad_0";
    e[GLFW_KEY_KP_1].key = "pad_1";
    e[GLFW_KEY_KP_2].key = "pad_2";
    e[GLFW_KEY_KP_3].key = "pad_3";
    e[GLFW_KEY_KP_4].key = "pad_4";
    e[GLFW_KEY_KP_5].key = "pad_5";
    e[GLFW_KEY_KP_6].key = "pad_6";
    e[GLFW_KEY_KP_7].key = "pad_7";
    e[GLFW_KEY_KP_8].key = "pad_8";
    e[GLFW_KEY_KP_9].key = "pad_9";
    e[GLFW_KEY_KP_DECIMAL].key = "pad_decimal";
    e[GLFW_KEY_KP_DIVIDE].key = "pad_divide";
    e[GLFW_KEY_KP_MULTIPLY].key = "pad_multiply";
    e[GLFW_KEY_KP_SUBTRACT].key = "pad_subtract";
    e[GLFW_KEY_KP_ADD].key = "pad_add";
    e[GLFW_KEY_KP_ENTER].key = "pad_enter";
    e[GLFW_KEY_KP_EQUAL].key = "pad_equal";
    e[GLFW_KEY_LEFT_SHIFT].key = "left_shift";
    e[GLFW_KEY_LEFT_CONTROL].key = "left_ctrl";
    e[GLFW_KEY_LEFT_ALT].key = "left_alt";
    e[GLFW_KEY_LEFT_SUPER].key = "left_super";
    e[GLFW_KEY_RIGHT_SHIFT].key = "right_shift";
    e[GLFW_KEY_RIGHT_CONTROL].key = "right_ctrl";
    e[GLFW_KEY_RIGHT_ALT].key = "right_alt";
    e[GLFW_KEY_RIGHT_SUPER].key = "right_super";
    e[GLFW_KEY_MENU].key = "menu";

    Py_XINCREF(self);
    return (PyObject *) self;
}

static PyObject *Key_getattro(Key *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return NULL;
    
    for (ushort i = 0; i <= GLFW_KEY_LAST; i ++)
        if (self -> keys[i].key && !strcmp(self -> keys[i].key, name))
            return (PyObject *) buttonNew(&self -> keys[i]);

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static PyTypeObject KeyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Key",
    .tp_doc = "the keyboard input handler",
    .tp_basicsize = sizeof(Key),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Key_new,
    .tp_getattro = (getattrofunc) Key_getattro,
    .tp_getset = KeyGetSetters
};

static PyObject *Camera_getX(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[0]);
}

static int Camera_setX(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> pos[0] = PyFloat_AsDouble(value);
    return self -> pos[0] == -1 && PyErr_Occurred() ? -1 : 0;
}

static PyObject *Camera_getY(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[1]);
}

static int Camera_setY(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> pos[1] = PyFloat_AsDouble(value);
    return self -> pos[1] == -1 && PyErr_Occurred() ? -1 : 0;
}

static vec Camera_vecPos(Camera *self) {
    return self -> pos;
}

static PyObject *Camera_getPos(Camera *self, void *Py_UNUSED(closure)) {
    Vector *pos = vectorNew((PyObject *) self, (method) Camera_vecPos, 2);
    pos -> data[0].set = (setter) Camera_setX;
    pos -> data[1].set = (setter) Camera_setY;
    pos -> data[0].name = "x";
    pos -> data[1].name = "y";

    return (PyObject *) pos;
}

static int Camera_setPos(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> pos, 2);
}

static PyObject *Camera_getTop(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[1] + getWindowSize()[1] / 2);
}

static PyObject *Camera_getBottom(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[1] - getWindowSize()[1] / 2);
}

static PyObject *Camera_getLeft(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[0] - getWindowSize()[0] / 2);
}

static PyObject *Camera_getRight(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[0] + getWindowSize()[0] / 2);
}

static PyGetSetDef CameraGetSetters[] = {
    {"x", (getter) Camera_getX, (setter) Camera_setX, "x position of the camera", NULL},
    {"y", (getter) Camera_getY, (setter) Camera_setY, "y position of the camera", NULL},
    {"position", (getter) Camera_getPos, (setter) Camera_setPos, "position of the camera", NULL},
    {"pos", (getter) Camera_getPos, (setter) Camera_setPos, "position of the camera", NULL},
    {"top", (getter) Camera_getTop, NULL, "top position of the camera", NULL},
    {"bottom", (getter) Camera_getBottom, NULL, "bottom position of the camera", NULL},
    {"left", (getter) Camera_getLeft, NULL, "left position of the camera", NULL},
    {"right", (getter) Camera_getRight, NULL, "right position of the camera", NULL},
    {NULL}
};

static PyObject *Camera_moveToward(Camera *self, PyObject *args) {
    if (moveToward(self -> pos, args))
        return NULL;

    Py_RETURN_NONE;
}

static PyObject *Camera_moveSmooth(Camera *self, PyObject *args) {
    if (moveSmooth(self -> pos, args))
        return NULL;

    Py_RETURN_NONE;
}

static PyMethodDef CameraMethods[] = {
    {"move_toward", (PyCFunction) Camera_moveToward, METH_VARARGS, "move the camera toward another object"},
    {"move_smooth", (PyCFunction) Camera_moveSmooth, METH_VARARGS, "move the camera smoothly toward another object"},
    {NULL}
};

static PyObject *Camera_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Camera *self = camera = (Camera *) type -> tp_alloc(type, 0);

    Py_XINCREF(self);
    return (PyObject *) self;
}

static int Camera_init(Camera *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", NULL};

    self -> pos[0] = 0;
    self -> pos[1] = 0;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|sddO", kwlist, &self -> pos[0],
        &self -> pos[1]) ? 0 : -1;
}

static PyTypeObject CameraType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Camera",
    .tp_doc = "the user screen view and projection",
    .tp_basicsize = sizeof(Camera),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Camera_new,
    .tp_init = (initproc) Camera_init,
    .tp_getset = CameraGetSetters,
    .tp_methods = CameraMethods
};

static void windowClearColor() {
    glClearColor(
        (float) window -> color[0], (float) window -> color[1],
        (float) window -> color[2], 1);
}

static PyObject *Window_getCaption(Window *self, void *Py_UNUSED(closure)) {
    return PyUnicode_FromString(self -> caption);
}

static int Window_setCaption(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const char *caption = PyUnicode_AsUTF8(value);
    if (!caption) return -1;

    free(self -> caption);
    self -> caption = strdup(caption);

    return glfwSetWindowTitle(self -> glfw, self -> caption), 0;
}

static PyObject *Window_getRed(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[0]);
}

static int Window_setRed(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[0] = PyFloat_AsDouble(value);
    return self -> color[0] == -1 && PyErr_Occurred() ? -1 : windowClearColor(), 0;
}

static PyObject *Window_getGreen(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[1]);
}

static int Window_setGreen(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[1] = PyFloat_AsDouble(value);
    return self -> color[1] == -1 && PyErr_Occurred() ? -1 : windowClearColor(), 0;
}

static PyObject *Window_getBlue(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[2]);
}

static int Window_setBlue(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[2] = PyFloat_AsDouble(value);
    return self -> color[2] == -1 && PyErr_Occurred() ? -1 : windowClearColor(), 0;
}

static vec Window_vecColor(Window *self) {
    return self -> color;
}

static PyObject *Window_getColor(Window *self, void *Py_UNUSED(closure)) {
    Vector *color = vectorNew((PyObject *) self, (method) Window_vecColor, 3);
    color -> data[0].set = (setter) Window_setRed;
    color -> data[1].set = (setter) Window_setGreen;
    color -> data[2].set = (setter) Window_setBlue;
    color -> data[0].name = "red";
    color -> data[1].name = "green";
    color -> data[2].name = "blue";

    return (PyObject *) color;
}

static int Window_setColor(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> color, 3) ? -1 : windowClearColor(), 0;
}

static PyObject *Window_getWidth(Window *Py_UNUSED(self), void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(getWindowSize()[0]);
}

static int Window_setWidth(Window *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const int width = (int) PyFloat_AsDouble(value);
    if (width == -1 && PyErr_Occurred()) return -1;

    START
    glfwSetWindowSize(window -> glfw, width, (int) getWindowSize()[1]);
    END return 0;
}

static PyObject *Window_getHeight(Window *Py_UNUSED(self), void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(getWindowSize()[1]);
}

static int Window_setHeight(Window *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const int height = (int) PyFloat_AsDouble(value);
    if (height == -1 && PyErr_Occurred()) return -1;

    START
    glfwSetWindowSize(window -> glfw, (int) getWindowSize()[0], height);
    END return 0;
}

static PyObject *Window_getSize(Window *self, void *Py_UNUSED(closure)) {
    Vector *size = vectorNew((PyObject *) self, (method) getWindowSize, 2);
    size -> data[0].name = "x";
    size -> data[1].name = "y";

    return (PyObject *) size;
}

static int Window_setSize(Window *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    vec size = getWindowSize();

    if (vectorSet(value, size, 2))
        return -1;

    START
    glfwSetWindowSize(window -> glfw, (int) size[0], (int) size[1]);
    END return 0;
}

static PyObject *Window_getResizable(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(glfwGetWindowAttrib(self -> glfw, GLFW_RESIZABLE));
}

static int Window_setResizable(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const int bool = PyObject_IsTrue(value);
    return bool == -1 ? -1 : glfwSetWindowAttrib(self -> glfw, GLFW_RESIZABLE, bool), 0;
}

static PyObject *Window_getDecorated(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(glfwGetWindowAttrib(self -> glfw, GLFW_DECORATED));
}

static int Window_setDecorated(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const int bool = PyObject_IsTrue(value);
    return bool == -1 ? -1 : glfwSetWindowAttrib(self -> glfw, GLFW_DECORATED, bool), 0;
}

static PyObject *Window_getResize(Window *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> resize);
}

static PyGetSetDef WindowGetSetters[] = {
    {"caption", (getter) Window_getCaption, (setter) Window_setCaption, "name of the window", NULL},
    {"red", (getter) Window_getRed, (setter) Window_setRed, "red color of the window", NULL},
    {"green", (getter) Window_getGreen, (setter) Window_setGreen, "green color of the window", NULL},
    {"blue", (getter) Window_getBlue, (setter) Window_setBlue, "blue color of the window", NULL},
    {"color", (getter) Window_getColor, (setter) Window_setColor, "color of the window", NULL},
    {"width", (getter) Window_getWidth, (setter) Window_setWidth, "width of the window", NULL},
    {"height", (getter) Window_getHeight, (setter) Window_setHeight, "height of the window", NULL},
    {"size", (getter) Window_getSize, (setter) Window_setSize, "dimensions of the window", NULL},
    {"resizable", (getter) Window_getResizable, (setter) Window_setResizable, "the window is resizable", NULL},
    {"decorated", (getter) Window_getDecorated, (setter) Window_setDecorated, "the window is decorated", NULL},
    {"resize", (getter) Window_getResize, NULL, "the window is resized", NULL},
    {NULL}
};

static PyObject *Window_close(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwSetWindowShouldClose(self -> glfw, GLFW_TRUE);
    Py_RETURN_NONE;
}

static PyObject *Window_maximize(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwMaximizeWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyObject *Window_minimize(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwIconifyWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyObject *Window_restore(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwRestoreWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyObject *Window_focus(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwFocusWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyMethodDef WindowMethods[] = {
    {"close", (PyCFunction) Window_close, METH_NOARGS, "close the window"},
    {"maximize", (PyCFunction) Window_maximize, METH_NOARGS, "maximize the window"},
    {"minimize", (PyCFunction) Window_minimize, METH_NOARGS, "minimize the window"},
    {"restore", (PyCFunction) Window_restore, METH_NOARGS, "restore the window from maximized or minimized"},
    {"focus", (PyCFunction) Window_focus, METH_NOARGS, "bring the window to the front"},
    {NULL}
};

static PyObject *Window_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Window *self = window = (Window *) type -> tp_alloc(type, 0);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    if (!(self -> glfw = glfwCreateWindow(640, 480, "JoBase", NULL, NULL))) {
        const char *buffer;
        glfwGetError(&buffer);

        PyErr_SetString(PyExc_OSError, buffer);
        return glfwTerminate(), NULL;
    }

    glfwMakeContextCurrent(self -> glfw);
    glfwSetWindowRefreshCallback(self -> glfw, windowRefreshCallback);
    glfwSetWindowSizeCallback(self -> glfw, windowSizeCallback);
    glfwSetFramebufferSizeCallback(self -> glfw, framebufferSizeCallback);
    glfwSetCursorPosCallback(self -> glfw, cursorPosCallback);
    glfwSetCursorEnterCallback(self -> glfw, cursorEnterCallback);
    glfwSetMouseButtonCallback(self -> glfw, mouseButtonCallback);
    glfwSetKeyCallback(self -> glfw, keyCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        PyErr_SetString(PyExc_OSError, "failed to load OpenGL");
        return glfwTerminate(), NULL;
    }

    Py_XINCREF(self);
    return (PyObject *) self;
}

static int Window_init(Window *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"caption", "width", "height", "color", NULL};
    const char *caption = "JoBase";

    PyObject *color = NULL;
    int2 size = {640, 480};

    self -> color[0] = 1;
    self -> color[1] = 1;
    self -> color[2] = 1;
    self -> resize = 1;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|siiO", kwlist, &caption,  &size[0], &size[1], &color) ||
        (color && vectorSet(color, self -> color, 3))) return -1;

    self -> caption = strdup(caption);
    glfwSetWindowTitle(self -> glfw, self -> caption);
    glfwSetWindowSize(self -> glfw, size[0], size[1]);

    return windowClearColor(), 0;
}

static void Window_dealloc(Window *self) {
    free(self -> caption);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyTypeObject WindowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Window",
    .tp_doc = "the main window",
    .tp_basicsize = sizeof(Window),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Window_new,
    .tp_init = (initproc) Window_init,
    .tp_dealloc = (destructor) Window_dealloc,
    .tp_methods = WindowMethods,
    .tp_getset = WindowGetSetters
};

static void baseSetPos(Base *self) {
    if (self -> body)
        cpBodySetPosition(self -> body, cpv(self -> pos[0], self -> pos[1]));
}

static void baseSetVelocity(Base *self) {
    if (self -> body)
        cpBodySetVelocity(self -> body, cpv(self -> velocity[0], self -> velocity[1]));
}

static void baseSetMoment(Base *self) {
    if (self -> body && self -> type == DYNAMIC)
        cpBodySetMoment(self -> body, self -> rotate ? self -> getMoment(self) : INFINITY);
}

static void baseSetAngle(Base *self) {
    if (self -> body)
        cpBodySetAngle(self -> body, self -> angle * M_PI / 180);
}

static void baseMatrix(Base *self, double x, double y) {
    const float sx = (float) (x * self -> scale[0]);
    const float sy = (float) (y * self -> scale[1]);
    const float ax = (float) self -> anchor[0];
    const float ay = (float) self -> anchor[1];
    const float px = (float) self -> pos[0];
    const float py = (float) self -> pos[1];
    const float s = (float) sin(self -> angle * M_PI / 180);
	const float c = (float) cos(self -> angle * M_PI / 180);

    mat matrix = {
        sx * c, sx * s, 0, 0,
        sy * -s, sy * c, 0, 0,
        0, 0, 1, 0,
        ax * c + ay * -s + px, ax * s + ay * c + py, 0, 1
    };

    setUniform(matrix, self -> color);
}

static void baseInit(Base *self) {
    self -> pos[0] = 0;
    self -> pos[1] = 0;

    self -> color[0] = 0;
    self -> color[1] = 0;
    self -> color[2] = 0;
    self -> color[3] = 1;

    self -> anchor[0] = 0;
    self -> anchor[1] = 0;

    self -> scale[0] = 1;
    self -> scale[1] = 1;

    self -> velocity[0] = 0;
    self -> velocity[1] = 0;

    self -> angle = 0;
    self -> angularVelocity = 0;

    self -> type = DYNAMIC;
    self -> mass = 1;
    self -> elasticity = .5;
    self -> friction = .5;
    self -> rotate = 1;
}

static PyObject *Base_getX(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[0]);
}

static int Base_setX(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> pos[0] = PyFloat_AsDouble(value);
    return self -> pos[0] == -1 && PyErr_Occurred() ? -1 : baseSetPos(self), 0;
}

static PyObject *Base_getY(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[1]);
}

static int Base_setY(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> pos[1] = PyFloat_AsDouble(value);
    return self -> pos[1] == -1 && PyErr_Occurred() ? -1 : baseSetPos(self), 0;
}

static vec Base_vecPos(Base *self) {
    return self -> pos;
}

static PyObject *Base_getPos(Base *self, void *Py_UNUSED(closure)) {
    Vector *pos = vectorNew((PyObject *) self, (method) Base_vecPos, 2);
    pos -> data[0].set = (setter) Base_setX;
    pos -> data[1].set = (setter) Base_setY;
    pos -> data[0].name = "x";
    pos -> data[1].name = "y";

    return (PyObject *) pos;
}

static int Base_setPos(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> pos, 2) ? -1 : baseSetPos(self), 0;
}

static int Base_setScaleX(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> scale[0] = PyFloat_AsDouble(value);
    return self -> scale[0] == -1 && PyErr_Occurred() ? -1 : self -> setBase(self), 0;
}

static int Base_setScaleY(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> scale[1] = PyFloat_AsDouble(value);
    return self -> scale[1] == -1 && PyErr_Occurred() ? -1 : self -> setBase(self), 0;
}

static vec Base_vecScale(Base *self) {
    return self -> scale;
}

static PyObject *Base_getScale(Base *self, void *Py_UNUSED(closure)) {
    Vector *scale = vectorNew((PyObject *) self, (method) Base_vecScale, 2);
    scale -> data[0].set = (setter) Base_setScaleX;
    scale -> data[1].set = (setter) Base_setScaleY;
    scale -> data[0].name = "x";
    scale -> data[1].name = "y";

    return (PyObject *) scale;
}

static int Base_setScale(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> scale, 2) ? -1 : self -> setBase(self), 0;
}

static int Base_setAnchorX(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> anchor[0] = PyFloat_AsDouble(value);
    return self -> anchor[0] == -1 && PyErr_Occurred() ? -1 : 0;
}

static int Base_setAnchorY(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> anchor[1] = PyFloat_AsDouble(value);
    return self -> anchor[1] == -1 && PyErr_Occurred() ? -1 : 0;
}

static vec Base_vecAnchor(Base *self) {
    return self -> anchor;
}

static PyObject *Base_getAnchor(Base *self, void *Py_UNUSED(closure)) {
    Vector *anchor = vectorNew((PyObject *) self, (method) Base_vecAnchor, 2);
    anchor -> data[0].set = (setter) Base_setAnchorX;
    anchor -> data[1].set = (setter) Base_setAnchorY;
    anchor -> data[0].name = "x";
    anchor -> data[1].name = "y";

    return (PyObject *) anchor;
}

static int Base_setAnchor(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> anchor, 2);
}

static PyObject *Base_getAngle(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> angle);
}

static int Base_setAngle(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> angle = PyFloat_AsDouble(value);
    return self -> angle == -1 && PyErr_Occurred() ? -1 : baseSetAngle(self), 0;
}

static PyObject *Base_getRed(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[0]);
}

static int Base_setRed(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[0] = PyFloat_AsDouble(value);
    return self -> color[0] == -1 && PyErr_Occurred() ? -1 : 0;
}

static PyObject *Base_getGreen(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[1]);
}

static int Base_setGreen(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[1] = PyFloat_AsDouble(value);
    return self -> color[1] == -1 && PyErr_Occurred() ? -1 : 0;
}

static PyObject *Base_getBlue(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[2]);
}

static int Base_setBlue(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[2] = PyFloat_AsDouble(value);
    return self -> color[2] == -1 && PyErr_Occurred() ? -1 : 0;
}

static PyObject *Base_getAlpha(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[3]);
}

static int Base_setAlpha(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> color[3] = PyFloat_AsDouble(value);
    return self -> color[3] == -1 && PyErr_Occurred() ? -1 : 0;
}

static vec Base_vecColor(Base *self) {
    return self -> color;
}

static PyObject *Base_getColor(Base *self, void *Py_UNUSED(closure)) {
    Vector *color = vectorNew((PyObject *) self, (method) Base_vecColor, 4);
    color -> data[1].set = (setter) Base_setRed;
    color -> data[3].set = (setter) Base_setGreen;
    color -> data[1].set = (setter) Base_setBlue;
    color -> data[3].set = (setter) Base_setAlpha;
    color -> data[0].name = "red";
    color -> data[1].name = "green";
    color -> data[2].name = "blue";
    color -> data[3].name = "alpha";

    return (PyObject *) color;
}

static int Base_setColor(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> color, 4);
}

static PyObject *Base_getLeft(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> getLeft(self));
}

static int Base_setLeft(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)
    
    const double result = PyFloat_AsDouble(value);
    if (result == -1 && PyErr_Occurred()) return -1;

    self -> pos[0] += result - self -> getLeft(self);
    return baseSetPos(self), 0;
}

static PyObject *Base_getTop(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> getTop(self));
}

static int Base_setTop(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)
    
    const double result = PyFloat_AsDouble(value);
    if (result == -1 && PyErr_Occurred()) return -1;

    self -> pos[1] += result - self -> getTop(self);
    return baseSetPos(self), 0;
}

static PyObject *Base_getRight(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> getRight(self));
}

static int Base_setRight(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)
    
    const double result = PyFloat_AsDouble(value);
    if (result == -1 && PyErr_Occurred()) return -1;

    self -> pos[0] += result - self -> getRight(self);
    return baseSetPos(self), 0;
}

static PyObject *Base_getBottom(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> getBottom(self));
}

static int Base_setBottom(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)
    
    const double result = PyFloat_AsDouble(value);
    if (result == -1 && PyErr_Occurred()) return -1;

    self -> pos[1] += result - self -> getBottom(self);
    return baseSetPos(self), 0;
}

static PyObject *Base_getType(Base *self, void *Py_UNUSED(closure)) {
    return PyLong_FromLong(self -> type);
}

static int Base_setType(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    if ((self -> type = PyLong_AsLong(value)) == -1 && PyErr_Occurred())
        return -1;

    if (self -> type != DYNAMIC && self -> type != STATIC) {
        PyErr_SetString(PyExc_ValueError, "type must be DYNAMIC or STATIC");
        return -1;
    }

    if (self -> body)
        cpBodySetType(self -> body, self -> type);

    return baseSetMoment(self), 0;
}

static PyObject *Base_getMass(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> mass);
}

static int Base_setMass(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    if ((self -> mass = PyFloat_AsDouble(value)) == -1 && PyErr_Occurred())
        return -1;

    if (self -> body)
        cpBodySetMass(self -> body, self -> mass);

    return baseSetMoment(self), 0;
}

static PyObject *Base_getElasticity(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> elasticity);
}

static int Base_setElasticity(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    if ((self -> elasticity = PyFloat_AsDouble(value)) == -1 && PyErr_Occurred())
        return -1;

    if (self -> shape)
        cpShapeSetElasticity(self -> shape, self -> elasticity);
        
    return 0;
}

static PyObject *Base_getFriction(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> friction);
}

static int Base_setFriction(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    if ((self -> friction = PyFloat_AsDouble(value)) == -1 && PyErr_Occurred())
        return -1;

    if (self -> shape)
        cpShapeSetFriction(self -> shape, self -> friction);
        
    return 0;
}

static int Base_setVelocityX(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> velocity[0] = PyFloat_AsDouble(value);
    return self -> velocity[0] == -1 && PyErr_Occurred() ? -1 : baseSetVelocity(self), 0;
}

static int Base_setVelocityY(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> velocity[1] = PyFloat_AsDouble(value);
    return self -> velocity[1] == -1 && PyErr_Occurred() ? -1 : baseSetVelocity(self), 0;
}

static vec Base_vecVelocity(Base *self) {
    return self -> velocity;
}

static PyObject *Base_getVelocity(Base *self, void *Py_UNUSED(closure)) {
    Vector *speed = vectorNew((PyObject *) self, (method) Base_vecVelocity, 2);
    speed -> data[0].set = (setter) Base_setVelocityX;
    speed -> data[1].set = (setter) Base_setVelocityY;
    speed -> data[0].name = "x";
    speed -> data[1].name = "y";

    return (PyObject *) speed;
}

static int Base_setVelocity(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> velocity, 2) ? -1 : baseSetVelocity(self), 0;
}

static PyObject *Base_getAngularVelocity(Base *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> angularVelocity);
}

static int Base_setAngularVelocity(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    if ((self -> angularVelocity = PyFloat_AsDouble(value)) == -1 && PyErr_Occurred())
        return -1;

    if (self -> body)
        cpBodySetAngularVelocity(self -> body, self -> angularVelocity * M_PI / 180);

    return 0;
}

static PyObject *Base_getRotate(Base *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> rotate);
}

static int Base_setRotate(Base *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const int result = PyObject_IsTrue(value);
    if (result == -1) return -1;
    
    self -> rotate = result;
    return baseSetMoment(self), 0;
}

static PyGetSetDef BaseGetSetters[] = {
    {"x", (getter) Base_getX, (setter) Base_setX, "x position of the object", NULL},
    {"y", (getter) Base_getY, (setter) Base_setY, "y position of the object", NULL},
    {"position", (getter) Base_getPos, (setter) Base_setPos, "position of the object", NULL},
    {"pos", (getter) Base_getPos, (setter) Base_setPos, "position of the object", NULL},
    {"scale", (getter) Base_getScale, (setter) Base_setScale, "scale of the object", NULL},
    {"anchor", (getter) Base_getAnchor, (setter) Base_setAnchor, "rotation origin of the object", NULL},
    {"angle", (getter) Base_getAngle, (setter) Base_setAngle, "angle of the object", NULL},
    {"red", (getter) Base_getRed, (setter) Base_setRed, "red color of the object", NULL},
    {"green", (getter) Base_getGreen, (setter) Base_setGreen, "green color of the object", NULL},
    {"blue", (getter) Base_getBlue, (setter) Base_setBlue, "blue color of the object", NULL},
    {"alpha", (getter) Base_getAlpha, (setter) Base_setAlpha, "opacity of the object", NULL},
    {"color", (getter) Base_getColor, (setter) Base_setColor, "color of the object", NULL},
    {"left", (getter) Base_getLeft, (setter) Base_setLeft, "left position of the object", NULL},
    {"top", (getter) Base_getTop, (setter) Base_setTop, "top position of the object", NULL},
    {"right", (getter) Base_getRight, (setter) Base_setRight, "right position of the object", NULL},
    {"bottom", (getter) Base_getBottom, (setter) Base_setBottom, "bottom position of the object", NULL},
    {"type", (getter) Base_getType, (setter) Base_setType, "physics body of the object", NULL},
    {"mass", (getter) Base_getMass, (setter) Base_setMass, "weight of the object", NULL},
    {"weight", (getter) Base_getMass, (setter) Base_setMass, "weight of the object", NULL},
    {"elasticity", (getter) Base_getElasticity, (setter) Base_setElasticity, "bounciness of the object", NULL},
    {"friction", (getter) Base_getFriction, (setter) Base_setFriction, "roughness of the object", NULL},
    {"velocity", (getter) Base_getVelocity, (setter) Base_setVelocity, "physics speed of the object", NULL},
    {"speed", (getter) Base_getVelocity, (setter) Base_setVelocity, "physics speed of the object", NULL},
    {"angular_velocity", (getter) Base_getAngularVelocity, (setter) Base_setAngularVelocity, "physics rotation speed of the object", NULL},
    {"rotate_speed", (getter) Base_getAngularVelocity, (setter) Base_setAngularVelocity, "physics rotation speed of the object", NULL},
    {"rotate", (getter) Base_getRotate, (setter) Base_setRotate, "the object is able to rotate in a physics engine", NULL},
    {NULL}
};

static PyObject *Base_lookAt(Base *self, PyObject *other) {
    vec2 pos;
    
    if (getOtherPos(other, pos))
        return NULL;

    const double angle = atan2(pos[1] - self -> pos[1], pos[0] - self -> pos[0]);
    self -> angle = angle * 180 / M_PI;
    
    baseSetAngle(self);
    Py_RETURN_NONE;
}

static PyObject *Base_moveToward(Base *self, PyObject *args) {
    if (moveToward(self -> pos, args))
        return NULL;

    baseSetPos(self);
    Py_RETURN_NONE;
}

static PyObject *Base_moveSmooth(Base *self, PyObject *args) {
    if (moveSmooth(self -> pos, args))
        return NULL;

    baseSetPos(self);
    Py_RETURN_NONE;
}

static PyObject *Base_applyImpulse(Base *self, PyObject *args) {
    if (!self -> body) {
        PyErr_SetString(PyExc_AttributeError, "must be added to a physics engine");
        return NULL;
    }

    cpVect point = {0, 0};
    cpVect impulse;

    if (!PyArg_ParseTuple(args, "dd|dd", &impulse.x, &impulse.y, &point.x, &point.y))
        return NULL;

    cpBodyApplyImpulseAtLocalPoint(self -> body, impulse, point);
    Py_RETURN_NONE;
}

static PyMethodDef BaseMethods[] = {
    {"collides_with", (PyCFunction) Object_collidesWith, METH_O, "check if the object collides with another object"},
    {"look_at", (PyCFunction) Base_lookAt, METH_O, "rotate the object so that it points to another object"},
    {"move_toward", (PyCFunction) Base_moveToward, METH_VARARGS, "move the object toward another object"},
    {"move_smooth", (PyCFunction) Base_moveSmooth, METH_VARARGS, "move the object smoothly toward another object"},
    {"apply_impulse", (PyCFunction) Base_applyImpulse, METH_VARARGS, "apply an impulse to the object physics"},
    {NULL}
};

static PyTypeObject BaseType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Base",
    .tp_doc = "base class for drawing things",
    .tp_basicsize = sizeof(Base),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_methods = BaseMethods,
    .tp_getset = BaseGetSetters
};

static void rectangleDraw(Rectangle *self, uchar type) {
    baseMatrix((Base *) self, self -> size[0], self -> size[1]);

    glBindVertexArray(mesh);
    glUniform1i(glGetUniformLocation(program, "image"), type);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

static void rectangleNewShape(Rectangle *self) {
    self -> base.shape = cpBoxShapeNew(
        self -> base.body, self -> size[0] * self -> base.scale[0],
        self -> size[1] * self -> base.scale[1], 0);
}

static void rectangleSetBase(Rectangle *self) {
    if (!self -> base.shape) return;

    const double x = self -> size[0] / 2;
    const double y = self -> size[1] / 2;

    cpTransform transform = cpTransformNew(
        self -> base.scale[0], 0, 0,
        self -> base.scale[1], 0, 0);

    cpVect data[4] = {{-x, y}, {x, y}, {x, -y}, {-x, -y}};
    cpPolyShapeSetVerts(self -> base.shape, 4, data, transform);
    baseSetMoment((Base *) self);
}

static cpFloat rectangleGetMoment(Rectangle *self) {
    return cpMomentForBox(
        self -> base.mass, self -> size[0] * self -> base.scale[0],
        self -> size[1] * self -> base.scale[1]);
}

static double rectangleGetTop(Rectangle *self) {
    vec2 poly[4];
    return getRectPoly(self, poly), getPolyTop(poly, 4);
}

static double rectangleGetBottom(Rectangle *self) {
    vec2 poly[4];
    return getRectPoly(self, poly), getPolyBottom(poly, 4);
}

static double rectangleGetLeft(Rectangle *self) {
    vec2 poly[4];
    return getRectPoly(self, poly), getPolyLeft(poly, 4);
}

static double rectangleGetRight(Rectangle *self) {
    vec2 poly[4];
    return getRectPoly(self, poly), getPolyRight(poly, 4);
}

static PyObject *Rectangle_getWidth(Rectangle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size[0]);
}

static int Rectangle_setWidth(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> size[0] = PyFloat_AsDouble(value);
    return self -> size[0] == -1 && PyErr_Occurred() ? -1 : rectangleSetBase(self), 0;
}

static PyObject *Rectangle_getHeight(Rectangle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size[1]);
}

static int Rectangle_setHeight(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> size[1] = PyFloat_AsDouble(value);
    return self -> size[1] == -1 && PyErr_Occurred() ? -1 : rectangleSetBase(self), 0;
}

static vec Rectangle_vecSize(Rectangle *self) {
    return self -> size;
}

static PyObject *Rectangle_getSize(Rectangle *self, void *Py_UNUSED(closure)) {
    Vector *size = vectorNew((PyObject *) self, (method) Rectangle_vecSize, 2);
    size -> data[0].set = (setter) Rectangle_setWidth;
    size -> data[1].set = (setter) Rectangle_setHeight;
    size -> data[0].name = "x";
    size -> data[1].name = "y";

    return (PyObject *) size;
}

static int Rectangle_setSize(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> size, 2) ? -1 : rectangleSetBase(self), 0;
}

static PyGetSetDef RectangleGetSetters[] = {
    {"width", (getter) Rectangle_getWidth, (setter) Rectangle_setWidth, "width of the rectangle", NULL},
    {"height", (getter) Rectangle_getHeight, (setter) Rectangle_setHeight, "height of the rectangle", NULL},
    {"size", (getter) Rectangle_getSize, (setter) Rectangle_setSize, "dimentions of the rectangle", NULL},
    {NULL}
};

static PyObject *Rectangle_draw(Rectangle *self, PyObject *Py_UNUSED(ignored)) {
    rectangleDraw(self, SHAPE);
    Py_RETURN_NONE;
}

static PyMethodDef RectangleMethods[] = {
    {"draw", (PyCFunction) Rectangle_draw, METH_NOARGS, "draw the rectangle on the screen"},
    {NULL}
};

static PyObject *Rectangle_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Rectangle *self = (Rectangle *) type -> tp_alloc(type, 0);

    self -> base.newShape = (void *)(Base *) rectangleNewShape;
    self -> base.setBase = (void *)(Base *) rectangleSetBase;
    self -> base.getMoment = (cpFloat (*)(Base *)) rectangleGetMoment;
    self -> base.getTop = (double (*)(Base *)) rectangleGetTop;
    self -> base.getBottom = (double (*)(Base *)) rectangleGetBottom;
    self -> base.getLeft = (double (*)(Base *)) rectangleGetLeft;
    self -> base.getRight = (double (*)(Base *)) rectangleGetRight;

    return (PyObject *) self;
}

static int Rectangle_init(Rectangle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "width", "height", "angle", "color", NULL};

    PyObject *color = NULL;
    baseInit((Base *) self);

    self -> size[0] = 50;
    self -> size[1] = 50;

    return !PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO", kwlist, &self -> base.pos[0], &self -> base.pos[1],
        &self -> size[0], &self -> size[1], &self -> base.angle,
        &color) || (color && vectorSet(color, self -> base.color, 4)) ? -1 : 0;
}

static PyTypeObject RectangleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Rectangle",
    .tp_doc = "draw rectangles on the screen",
    .tp_basicsize = sizeof(Rectangle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Rectangle_new,
    .tp_init = (initproc) Rectangle_init,
    .tp_methods = RectangleMethods,
    .tp_getset = RectangleGetSetters
};

static PyObject *Image_draw(Image *self, PyObject *Py_UNUSED(ignored)) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self -> texture -> source);

    rectangleDraw(&self -> rect, IMAGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    Py_RETURN_NONE;
}

static PyMethodDef ImageMethods[] = {
    {"draw", (PyCFunction) Image_draw, METH_NOARGS, "draw the image on the screen"},
    {NULL}
};

static int Image_init(Image *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", "x", "y", "angle", "width", "height", NULL};
    const char *name = constructFilepath("images/man.png");

    vec2 size = {0};
    baseInit((Base *) self);

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sdddddO", kwlist, &name, &self -> rect.base.pos[0],
        &self -> rect.base.pos[1], &self -> rect.base.angle, &size[0],
        &size[1])) return -1;

    self -> rect.base.color[0] = 1;
    self -> rect.base.color[1] = 1;
    self -> rect.base.color[2] = 1;

    ITER(Texture, textures)
        if (!strcmp(this -> name, name)) {
            self -> texture = this;
            self -> rect.size[0] = size[0] ? size[0] : this -> size[0];
            self -> rect.size[1] = size[1] ? size[1] : this -> size[1];
            return 0;
        }

    int width, height;
    uchar *image = stbi_load(name, &width, &height, 0, STBI_rgb_alpha);
    if (!image) return errorFormat(PyExc_FileNotFoundError, "failed to load image: \"%s\"", name), -1;

    NEW(Texture, textures)
    glGenTextures(1, &textures -> source);
    glBindTexture(GL_TEXTURE_2D, textures -> source);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    setTextureParameters();
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    self -> rect.size[0] = size[0] ? size[0] : width;
    self -> rect.size[1] = size[1] ? size[1] : height;
    textures -> size[0] = width;
    textures -> size[1] = height;
    textures -> name = strdup(name);
    self -> texture = textures;

    return 0;
}

static PyTypeObject ImageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Image",
    .tp_doc = "draw images on the screen",
    .tp_basicsize = sizeof(Image),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &RectangleType,
    .tp_new = Rectangle_new,
    .tp_init = (initproc) Image_init,
    .tp_methods = ImageMethods
};

static int textReset(Text *text) {
    if (FT_Set_Pixel_Sizes(text -> font -> face, (FT_UInt) text -> fontSize, 0))
        return errorFormat(PyExc_RuntimeError, "failed to set font size"), -1;

    text -> descender = text -> font -> face -> size -> metrics.descender >> 6;
    text -> base[1] = text -> font -> face -> size -> metrics.height >> 6;
    text -> base[0] = 0;

    PARSE(text -> content) {
        FT_UInt index = FT_Get_Char_Index(text -> font -> face, item);
        Char *glyph = &text -> chars[index];

        if (glyph -> fontSize != (int) text -> fontSize) {
            if (FT_Load_Glyph(text -> font -> face, index, FT_LOAD_DEFAULT))
                return errorFormat(PyExc_RuntimeError, "failed to load glyph: \"%lc\"", item), -1;

            if (FT_Render_Glyph(text -> font -> face -> glyph, FT_RENDER_MODE_NORMAL))
                return errorFormat(PyExc_RuntimeError, "failed to render glyph: \"%lc\"", item), -1;

            glyph -> advance = text -> font -> face -> glyph -> metrics.horiAdvance >> 6;
            glyph -> size[0] = text -> font -> face -> glyph -> metrics.width >> 6;
            glyph -> size[1] = text -> font -> face -> glyph -> metrics.height >> 6;
            glyph -> pos[0] = text -> font -> face -> glyph -> metrics.horiBearingX >> 6;
            glyph -> pos[1] = text -> font -> face -> glyph -> metrics.horiBearingY >> 6;
            glyph -> loaded ? glDeleteTextures(1, &glyph -> source) : (glyph -> loaded = 1);

            glGenTextures(1, &glyph -> source);
            glBindTexture(GL_TEXTURE_2D, glyph -> source);

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RED, glyph -> size[0], glyph -> size[1], 0, GL_RED,
                GL_UNSIGNED_BYTE, text -> font -> face -> glyph -> bitmap.buffer);

            setTextureParameters();
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (!i) text -> base[0] += glyph -> pos[0];
        if (text -> content[i + 1]) text -> base[0] += glyph -> advance;
        else text -> base[0] += glyph -> size[0] + glyph -> pos[0];
    }

    text -> rect.size[0] = text -> base[0];
    text -> rect.size[1] = text -> base[1];

    return 0;
    #undef ERROR
}

static void textAllocate(Text *text, Font *font) {
    text -> chars = realloc(text -> chars, font -> face -> num_glyphs * sizeof(Char));
    text -> font = font;

    for (uint i = 0; i < font -> face -> num_glyphs; i ++)
        text -> chars[i].loaded = 0;
}

static void textDelete(Text *text) {
    for (uint i = 0; i < text -> font -> face -> num_glyphs; i ++)
        if (text -> chars[i].loaded)
            glDeleteTextures(1, &text -> chars[i].source);
}

static int textResetFont(Text *text, const char *name) {
    FT_Face face;

    ITER(Font, fonts)
        if (!strcmp(this -> name, name))
            return textAllocate(text, this), 0;

    if (FT_New_Face(library, name, 0, &face))
        return errorFormat(PyExc_FileNotFoundError, "failed to load font: \"%s\"", name), -1;

    NEW(Font, fonts)
    fonts -> name = strdup(name);
    fonts -> face = face;

    return textAllocate(text, fonts), 0;
}

static PyObject *Text_getContent(Text *self, void *Py_UNUSED(closure)) {
    return PyUnicode_FromWideChar(self -> content, -1);
}

static int Text_setContent(Text *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    wchar_t *content = PyUnicode_AsWideCharString(value, NULL);
    if (!content) return -1;

    free(self -> content);
    return self -> content = wcsdup(content), textReset(self);
}

static PyObject *Text_getFont(Text *self, void *Py_UNUSED(closure)) {
    return PyUnicode_FromString(self -> font -> name);
}

static int Text_setFont(Text *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)
    textDelete(self);

    const char *name = PyUnicode_AsUTF8(value);
    return !name || textResetFont(self, name) ? -1 : textReset(self);
}

static PyObject *Text_getFontSize(Text *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> fontSize);
}

static int Text_setFontSize(Text *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> fontSize = PyFloat_AsDouble(value);
    return self -> fontSize == -1 && PyErr_Occurred() ? -1 : textReset(self);
}

static PyGetSetDef TextGetSetters[] = {
    {"content", (getter) Text_getContent, (setter) Text_setContent, "message of the text", NULL},
    {"font", (getter) Text_getFont, (setter) Text_setFont, "file path to the font family", NULL},
    {"font_size", (getter) Text_getFontSize, (setter) Text_setFontSize, "size of the font", NULL},
    {NULL}
};

static PyObject *Text_draw(Text *self, PyObject *Py_UNUSED(ignored)) {
    double pen = self -> rect.base.anchor[0] - self -> base[0] / 2;

    const float sx = (float) (self -> rect.base.scale[0] + self -> rect.size[0] / self -> base[0]) - 1;
    const float sy = (float) (self -> rect.base.scale[1] + self -> rect.size[1] / self -> base[1]) - 1;
    const float s = (float) sin(self -> rect.base.angle * M_PI / 180);
	const float c = (float) cos(self -> rect.base.angle * M_PI / 180);
    const float px = (float) self -> rect.base.pos[0];
    const float py = (float) self -> rect.base.pos[1];

    glUniform1i(glGetUniformLocation(program, "image"), TEXT);
    glBindVertexArray(mesh);

    PARSE(self -> content) {
        Char glyph = self -> chars[FT_Get_Char_Index(self -> font -> face, item)];
        if (!i) pen -= glyph.pos[0];

        const float ix = (float) glyph.size[0];
        const float iy = (float) glyph.size[1];

        const float ax = (float) (pen + glyph.pos[0] + glyph.size[0] / 2);
        const float ay = (float) (self -> rect.base.anchor[1] + glyph.pos[1] - (
            glyph.size[1] + self -> base[1]) / 2 - self -> descender);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glyph.source);

        mat matrix = {
            ix * sx * c, ix * sx * s, 0, 0,
            iy * sy * -s, iy * sy * c, 0, 0,
            0, 0, 1, 0,
            ax * sx * c + ay * sy * -s + px, ax * sx * s + ay * sy * c + py, 0, 1
        };

        setUniform(matrix, self -> rect.base.color);
        pen += glyph.advance;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
    Py_RETURN_NONE;
}

static PyMethodDef TextMethods[] = {
    {"draw", (PyCFunction) Text_draw, METH_NOARGS, "draw the text on the screen"},
    {NULL}
};

static int Text_init(Text *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"content", "x", "y", "font_size", "angle", "color", "font", NULL};
    const char *font = constructFilepath("fonts/default.ttf");

    PyObject *content = NULL;
    PyObject *color = NULL;

    baseInit((Base *) self);
    self -> fontSize = 50;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|UddddOs", kwlist, &content, &self -> rect.base.pos[0],
        &self -> rect.base.pos[1], &self -> fontSize, &self -> rect.base.angle, &color,
        &font) || textResetFont(self, font) || (color && vectorSet(
            color, self -> rect.base.color, 4))) return -1;

    if (content) {
        wchar_t *text = PyUnicode_AsWideCharString(content, NULL);
        if (!text) return -1;

        self -> content = wcsdup(text);
    }

    else self -> content = wcsdup(L"Text");
    return textReset(self);
}

static void Text_dealloc(Text *self) {
    if (self -> font) textDelete(self);

    free(self -> chars);
    free(self -> content);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyTypeObject TextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Text",
    .tp_doc = "draw text on the screen",
    .tp_basicsize = sizeof(Text),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &RectangleType,
    .tp_new = Rectangle_new,
    .tp_init = (initproc) Text_init,
    .tp_dealloc = (destructor) Text_dealloc,
    .tp_getset = TextGetSetters,
    .tp_methods = TextMethods
};

static uint circleGetVertices(Circle *self) {
    return (int) (sqrt(fabs(self -> radius * AVERAGE(self -> base.scale))) * 4) + 4;
}

static void circleSetData(Circle *self) {
    uint vertices = circleGetVertices(self) - 2;
    uint size = circleGetVertices(self) * 8;

    float *data = malloc(size);
    data[0] = 0;
    data[1] = 0;

    for (uint i = 0; i <= vertices; i ++) {
        const float angle = (float) M_PI * 2 * i / vertices;

        data[i * 2 + 2] = cosf(angle) / 2;
        data[i * 2 + 3] = sinf(angle) / 2;
    }

    glBindVertexArray(self -> vao);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
}

static void circleNewShape(Circle *self) {
    self -> base.shape = cpCircleShapeNew(
        self -> base.body, self -> radius * AVERAGE(self -> base.scale), cpv(0, 0));
}

static void circleSetBase(Circle *self) {
    circleSetData(self);

    if (self -> base.shape) {
        baseSetMoment((Base *) self);
        cpCircleShapeSetRadius(self -> base.shape, self -> radius * AVERAGE(self -> base.scale));
    }
}

static cpFloat circleGetMoment(Circle *self) {
    return cpMomentForCircle(
        self -> base.mass, 0, self -> radius * AVERAGE(self -> base.scale), cpv(0, 0));
}

static double circleGetTop(Circle *self) {
    return getBaseCenter((Base *) self)[1] + self -> radius * AVERAGE(self -> base.scale);
}

static double circleGetBottom(Circle *self) {
    return getBaseCenter((Base *) self)[1] - self -> radius * AVERAGE(self -> base.scale);
}

static double circleGetLeft(Circle *self) {
    return getBaseCenter((Base *) self)[0] - self -> radius * AVERAGE(self -> base.scale);
}

static double circleGetRight(Circle *self) {
    return getBaseCenter((Base *) self)[0] + self -> radius * AVERAGE(self -> base.scale);
}

static PyObject *Circle_getDiameter(Circle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> radius * 2);
}

static int Circle_setDiameter(Circle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const double diameter = PyFloat_AsDouble(value);
    if (diameter == -1 && PyErr_Occurred()) return -1;

    self -> radius = diameter / 2;
    return circleSetData(self), 0;
}

static PyObject *Circle_getRadius(Circle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> radius);
}

static int Circle_setRadius(Circle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    self -> radius = PyFloat_AsDouble(value);
    return self -> radius == -1 && PyErr_Occurred() ? -1 : circleSetData(self), 0;
}

static PyGetSetDef CircleGetSetters[] = {
    {"diameter", (getter) Circle_getDiameter, (setter) Circle_setDiameter, "diameter of the circle", NULL},
    {"radius", (getter) Circle_getRadius, (setter) Circle_setRadius, "radius of the circle", NULL},
    {NULL}
};

static PyObject *Circle_draw(Circle *self, PyObject *Py_UNUSED(ignored)) {
    baseMatrix((Base *) self, self -> radius * 2, self -> radius * 2);

    glBindVertexArray(self -> vao);
    glUniform1i(glGetUniformLocation(program, "image"), SHAPE);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circleGetVertices(self));
    glBindVertexArray(0);

    Py_RETURN_NONE;
}

static PyMethodDef CircleMethods[] = {
    {"draw", (PyCFunction) Circle_draw, METH_NOARGS, "draw the circle on the screen"},
    {NULL}
};

static PyObject *Circle_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Circle *self = (Circle *) type -> tp_alloc(type, 0);

    self -> base.newShape = (void *)(Base *) circleNewShape;
    self -> base.setBase = (void *)(Base *) circleSetBase;
    self -> base.getMoment = (cpFloat (*)(Base *)) circleGetMoment;
    self -> base.getTop = (double (*)(Base *)) circleGetTop;
    self -> base.getBottom = (double (*)(Base *)) circleGetBottom;
    self -> base.getLeft = (double (*)(Base *)) circleGetLeft;
    self -> base.getRight = (double (*)(Base *)) circleGetRight;

    glGenVertexArrays(1, &self -> vao);
    glBindVertexArray(self -> vao);
    glGenBuffers(1, &self -> vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);

    glVertexAttribPointer(glGetAttribLocation(program, "vertex"), 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return (PyObject *) self;
}

static int Circle_init(Circle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "diameter", "color", NULL};

    PyObject *color = NULL;
    double diameter = 50;
    baseInit((Base *) self);

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddO", kwlist, &self -> base.pos[0], &self -> base.pos[1], &diameter,
        &color) || (color && vectorSet(color, self -> base.color, 4))) return -1;

    self -> radius = diameter / 2;
    return circleSetData(self), 0;
}

static void Circle_dealloc(Circle *self) {
    glDeleteBuffers(1, &self -> vbo);
    glDeleteVertexArrays(1, &self -> vao);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyTypeObject CircleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Circle",
    .tp_doc = "draw circles on the screen",
    .tp_basicsize = sizeof(Circle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Circle_new,
    .tp_init = (initproc) Circle_init,
    .tp_dealloc = (destructor) Circle_dealloc,
    .tp_methods = CircleMethods,
    .tp_getset = CircleGetSetters
};

static cpVect *shapeGetPoly(Shape *self) {
    cpVect *vertices = malloc(self -> vertex * 16);

    for (uint i = 0; i < self -> vertex; i ++) {
        vertices[i].x = self -> points[i][0];
        vertices[i].y = self -> points[i][1];
    }

    return vertices;
}

static void shapeNewShape(Shape *self) {
    cpVect *vertices = shapeGetPoly(self);

    cpTransform transform = cpTransformNew(
        self -> base.scale[0], 0, 0,
        self -> base.scale[1], 0, 0);

    self -> base.shape = cpPolyShapeNew(self -> base.body, self -> vertex, vertices, transform, 0);
    free(vertices);
}

static void shapeSetBase(Shape *self) {
    if (!self -> base.shape) return;
    cpVect *vertices = shapeGetPoly(self);

    cpTransform transform = cpTransformNew(
        self -> base.scale[0], 0, 0,
        self -> base.scale[1], 0, 0);

    cpPolyShapeSetVerts(self -> base.shape, self -> vertex, vertices, transform);
    baseSetMoment((Base *) self);
    free(vertices);
}

static cpFloat shapeGetMoment(Shape *self) {
    cpVect *vertices = shapeGetPoly(self);
    cpFloat moment = cpMomentForPoly(self -> base.mass, self -> vertex, vertices, cpv(0, 0), 0);

    return free(vertices), moment;
}

static double shapeGetTop(Shape *self) {
    vec2 *poly = getShapePoly(self);

    const double value = getPolyTop(poly, self -> vertex);
    return free(poly), value;
}

static double shapeGetBottom(Shape *self) {
    vec2 *poly = getShapePoly(self);

    const double value = getPolyBottom(poly, self -> vertex);
    return free(poly), value;
}

static double shapeGetLeft(Shape *self) {
    vec2 *poly = getShapePoly(self);

    const double value = getPolyLeft(poly, self -> vertex);
    return free(poly), value;
}

static double shapeGetRight(Shape *self) {
    vec2 *poly = getShapePoly(self);

    const double value = getPolyRight(poly, self -> vertex);
    return free(poly), value;
}

static PyObject *Shape_draw(Shape *self, PyObject *Py_UNUSED(ignored)) {
    baseMatrix((Base *) self, 1, 1);

    glBindVertexArray(self -> vao);
    glUniform1i(glGetUniformLocation(program, "image"), SHAPE);
    glDrawElements(GL_TRIANGLES, self -> index, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    Py_RETURN_NONE;
}

static PyMethodDef ShapeMethods[] = {
    {"draw", (PyCFunction) Shape_draw, METH_NOARGS, "draw the shape on the screen"},
    {NULL}
};

static PyObject *Shape_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Shape *self = (Shape *) type -> tp_alloc(type, 0);

    self -> base.newShape = (void *)(Base *) shapeNewShape;
    self -> base.setBase = (void *)(Base *) shapeSetBase;
    self -> base.getMoment = (cpFloat (*)(Base *)) shapeGetMoment;
    self -> base.getTop = (double (*)(Base *)) shapeGetTop;
    self -> base.getBottom = (double (*)(Base *)) shapeGetBottom;
    self -> base.getLeft = (double (*)(Base *)) shapeGetLeft;
    self -> base.getRight = (double (*)(Base *)) shapeGetRight;

    self -> points = malloc(0);
    self -> indices = malloc(0);

    glGenVertexArrays(1, &self -> vao);
    glBindVertexArray(self -> vao);

    glGenBuffers(1, &self -> vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);

    glVertexAttribPointer(
        glGetAttribLocation(program, "vertex"),
        2, GL_DOUBLE, GL_FALSE, 0, 0);

    glGenBuffers(1, &self -> ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return (PyObject *) self;
}

static int Shape_init(Shape *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "x", "y", "angle", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;
    baseInit((Base *) self);

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|OdddO", kwlist, &points,
        &self -> base.pos[0], &self -> base.pos[1], &self -> base.angle,
        &color) || (color && vectorSet(color, self -> base.color, 4))) return -1;

    if (points) {
        if (!PySequence_Check(points)) ARRAY(points)
        PyObject *sequence = PySequence_Fast(points, NULL);

        self -> vertex = PySequence_Fast_GET_SIZE(sequence);
        self -> points = realloc(self -> points, self -> vertex * 16);

        if (self -> vertex < 3) {
            PyErr_SetString(PyExc_ValueError, "shape must have at least 3 corners");
            Py_DECREF(sequence);
            return -1;
        }

        for (uint i = 0; i < self -> vertex; i ++) {
            PyObject *point = PySequence_Fast_GET_ITEM(sequence, i);

            if (!PySequence_Check(point)) ARRAY(point)
            PyObject *value = PySequence_Fast(point, NULL);

            if (PySequence_Fast_GET_SIZE(value) < 2) {
                PyErr_SetString(PyExc_ValueError, "point must contain 2 values");
                Py_DECREF(sequence);
                Py_DECREF(value);
                return -1;
            }

            self -> points[i][0] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(value, 0));
            self -> points[i][1] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(value, 1));
            Py_DECREF(value);

            if ((self -> points[i][0] == -1 || self -> points[i][1] == -1) && PyErr_Occurred()) {
                Py_DECREF(sequence);
                return -1;
            }
        }

        Py_DECREF(sequence);

        if (triangulatePoly(self -> points, self -> vertex, &self -> indices, &self -> index))
            return -1;
    }

    else {
        self -> vertex = 3;
        self -> index = 3;

        self -> points = realloc(self -> points, self -> vertex * 16);
        self -> indices = realloc(self -> indices, self -> index * 4);

        self -> points[0][0] = 0;
        self -> points[0][1] = self -> points[1][0] = 25;
        self -> points[1][1] = self -> points[2][0] = self -> points[2][1] = -25;

        self -> indices[0] = 0;
        self -> indices[1] = 1;
        self -> indices[2] = 2;
    }

    glBindVertexArray(self -> vao);
    glBufferData(GL_ARRAY_BUFFER, self -> vertex * 16, self -> points, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, self -> index * 4, self -> indices, GL_STATIC_DRAW);
    return glBindVertexArray(0), 0;
}

static void Shape_dealloc(Shape *self) {
    uint buffers[] = {self -> vbo, self -> ibo};

    free(self -> indices);
    free(self -> points);

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyTypeObject ShapeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Shape",
    .tp_doc = "draw polygons on the screen",
    .tp_basicsize = sizeof(Shape),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Shape_new,
    .tp_init = (initproc) Shape_init,
    .tp_dealloc = (destructor) Shape_dealloc,
    .tp_methods = ShapeMethods
};

static void physicsRemove(Physics *self, uint index) {
    Base *object = self -> data[index];

    cpSpaceRemoveBody(self -> space, object -> body);
    cpSpaceRemoveShape(self -> space, object -> shape);
    cpBodyFree(object -> body);
    cpShapeFree(object -> shape);

    object -> body = NULL;
    object -> shape = NULL;
    Py_DECREF(object);
}

static Py_ssize_t Physics_len(Physics *self) {
    return self -> length;
}

static PyObject *Physics_item(Physics *self, Py_ssize_t index) {
    if (index >= self -> length)
        return PyErr_SetString(PyExc_IndexError, "index out of range"), NULL;

    return Py_INCREF(self -> data[index]), (PyObject *) self -> data[index];
}

static PySequenceMethods PhysicsSequenceMethods = {
    .sq_length = (lenfunc) Physics_len,
    .sq_item = (ssizeargfunc) Physics_item
};

static int Physics_setGravityX(Physics *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const double x = PyFloat_AsDouble(value);
    if (x == -1) return -1;

    cpSpaceSetGravity(self -> space, cpv(x, cpSpaceGetGravity(self -> space).y));
    return 0;
}

static int Physics_setGravityY(Physics *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value)

    const double y = PyFloat_AsDouble(value);
    if (y == -1) return -1;

    cpSpaceSetGravity(self -> space, cpv(cpSpaceGetGravity(self -> space).x, y));
    return 0;
}

static vec Physics_vecGravity(Physics *self) {
    static vec2 gravity;
    cpVect vector = cpSpaceGetGravity(self -> space);

    gravity[0] = vector.x;
    gravity[1] = vector.y;
    return gravity;
}

static PyObject *Physics_getGravity(Physics *self, void *Py_UNUSED(closure)) {
    Vector *gravity = vectorNew((PyObject *) self, (method) Physics_vecGravity, 2);
    gravity -> data[0].set = (setter) Physics_setGravityX;
    gravity -> data[1].set = (setter) Physics_setGravityY;
    gravity -> data[0].name = "x";
    gravity -> data[1].name = "y";

    return (PyObject *) gravity;
}

static int Physics_setGravity(Physics *self, PyObject *value, void *Py_UNUSED(closure)) {
    vec gravity = Physics_vecGravity(self);

    return vectorSet(value, gravity, 2) ? -1 : cpSpaceSetGravity(
        self -> space, cpv(gravity[0], gravity[1])), 0;
}

static PyGetSetDef PhysicsGetSetters[] = {
    {"gravity", (getter) Physics_getGravity, (setter) Physics_setGravity, "the gravity of the physics engine", NULL},
    {NULL}
};

static PyObject *Physics_add(Physics *self, PyObject *args) {
    Base *base;

    if (!PyArg_ParseTuple(args, "O!", &BaseType, &base))
        return NULL;

    if (base -> shape) {
        PyErr_SetString(PyExc_ValueError, "already added to a physics engine");
        return NULL;
    }

    base -> body = base -> type == DYNAMIC ? cpBodyNew(
        base -> mass, base -> rotate ? base -> getMoment(base) : INFINITY) :
            cpBodyNewKinematic();

    cpBodySetAngle(base -> body, base -> angle * M_PI / 180);
    cpBodySetPosition(base -> body, cpv(base -> pos[0], base -> pos[1]));
    cpBodySetVelocity(base -> body, cpv(base -> velocity[0], base -> velocity[1]));
    cpBodySetAngularVelocity(base -> body, base -> angularVelocity * M_PI / 180);

    base -> newShape(base);
    cpShapeSetElasticity(base -> shape, base -> elasticity);
    cpShapeSetFriction(base -> shape, base -> friction);

    cpSpaceAddBody(self -> space, base -> body);
    cpSpaceAddShape(self -> space, base -> shape);

    self -> data = realloc(self -> data, sizeof(Base *) * (self -> length + 1));
    self -> data[self -> length] = base;
    self -> length ++;

    Py_INCREF(base);
    Py_RETURN_NONE;
}

static PyObject *Physics_remove(Physics *self, PyObject *args) {
    Base *other;

    if (!PyArg_ParseTuple(args, "O!", &BaseType, &other))
        return NULL;

    for (uint i = 0; i < self -> length; i ++)
        if (self -> data[i] == other) {
            physicsRemove(self, i);
            self -> length --;

            for (uint j = i; j < self -> length; j ++)
                self -> data[j] = self -> data[j + 1];

            self -> data = realloc(self -> data, sizeof(Base *) * self -> length);
            Py_RETURN_NONE;
        }

    PyErr_SetString(PyExc_ValueError, "can't remove because it doesn't exist in physics engine");
    return NULL;
}

static PyObject *Physics_update(Physics *self, PyObject *Py_UNUSED(ignored)) {
    cpSpaceStep(self -> space, 1. / 60);

    for (uint i = 0; i < self -> length; i ++) {
        cpVect pos = cpBodyGetPosition(self -> data[i] -> body);
        cpVect velocity = cpBodyGetVelocity(self -> data[i] -> body);
        cpFloat angle = cpBodyGetAngle(self -> data[i] -> body);
        cpFloat angularVelocity = cpBodyGetAngularVelocity(self -> data[i] -> body);

        self -> data[i] -> pos[0] = pos.x;
        self -> data[i] -> pos[1] = pos.y;
        self -> data[i] -> velocity[0] = velocity.x;
        self -> data[i] -> velocity[1] = velocity.y;
        self -> data[i] -> angle = angle * 180 / M_PI;
        self -> data[i] -> angularVelocity = angularVelocity * 180 / M_PI;
    }
    
    Py_RETURN_NONE;
}

static PyMethodDef PhysicsMethods[] = {
    {"add", (PyCFunction) Physics_add, METH_VARARGS, "add an object to the physics engine"},
    {"remove", (PyCFunction) Physics_remove, METH_VARARGS, "remove an object from the physics engine"},
    {"update", (PyCFunction) Physics_update, METH_NOARGS, "update the physics step"},
    {NULL}
};

static PyObject *Physics_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Physics *self = (Physics *) type -> tp_alloc(type, 0);

    self -> data = realloc(self -> data, 0);
    self -> space = cpSpaceNew();

    return (PyObject *) self;
}

static int Physics_init(Physics *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"gravity_x", "gravity_y", NULL};
    cpVect vector = {0, -500};

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|dd", kwlist, &vector.x, &vector.y)) return -1;

    for (uint i = 0; i < self -> length; i ++)
        physicsRemove(self, i);

    self -> data = realloc(self -> data, 0);
    self -> length = 0;

    cpSpaceSetGravity(self -> space, vector);
    return 0;
}

static void Physics_dealloc(Physics *self) {
    for (uint i = 0; i < self -> length; i ++)
        physicsRemove(self, i);

    free(self -> data);
    cpSpaceFree(self -> space);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyTypeObject PhysicsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Physics",
    .tp_doc = "the physics engine",
    .tp_basicsize = sizeof(Physics),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Physics_new,
    .tp_init = (initproc) Physics_init,
    .tp_dealloc = (destructor) Physics_dealloc,
    .tp_getset = PhysicsGetSetters,
    .tp_methods = PhysicsMethods,
    .tp_as_sequence = &PhysicsSequenceMethods
};

static PyObject *Module_random(PyObject *Py_UNUSED(self), PyObject *args) {
    double x, y;

    return PyArg_ParseTuple(args, "dd", &x, &y) ? PyFloat_FromDouble(
        rand() / (RAND_MAX / fabs(y - x)) + (y < x ? y : x)) : NULL;
}

static PyObject *Module_randint(PyObject *Py_UNUSED(self), PyObject *args) {
    int x, y;

    if (!PyArg_ParseTuple(args, "ii", &x, &y))
        return NULL;

    return PyLong_FromLong(rand() / (RAND_MAX / abs(y - x + 1)) + (y < x ? y : x));
}

static PyObject *Module_run(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(ignored)) {
    PyObject *module = PyDict_GetItemString(PySys_GetObject("modules"), "__main__");

    if (PyObject_HasAttrString(module, "loop") && !(loop = PyObject_GetAttrString(module, "loop")))
        return NULL;

    glfwShowWindow(window -> glfw);

    while (!glfwWindowShouldClose(window -> glfw)) {
        if (PyErr_Occurred() || mainLoop()) return NULL;
        glfwPollEvents();
    }

    Py_RETURN_NONE;
}

static PyMethodDef ModuleMethods[] = {
    {"random", Module_random, METH_VARARGS, "find a random number between two numbers"},
    {"randint", Module_randint, METH_VARARGS, "find a random integer between two integers"},
    {"run", Module_run, METH_NOARGS, "activate the main game loop"},
    {NULL}
};

static int Module_exec(PyObject *self) {
    if (!glfwInit()) {
        const char *buffer;
        glfwGetError(&buffer);

        PyErr_SetString(PyExc_OSError, buffer);
        Py_DECREF(self);
        return -1;
    }

    if (FT_Init_FreeType(&library)) {
        PyErr_SetString(PyExc_OSError, "failed to initialize FreeType");
        Py_DECREF(self);
        return-1;
    }

    Py_ssize_t size;
    PyObject *file = PyObject_GetAttrString(self, "__file__");

    if (!file) {
        Py_DECREF(self);
        return -1;
    }

    path = (char *) PyUnicode_AsUTF8AndSize(file, &size);
    Py_DECREF(file);

    if (!path) {
        Py_DECREF(self);
        return -1;
    }

    char *last = strrchr(path, 47);
    length = size - strlen(last ? last : strrchr(path, 92)) + 1;
    path[length] = 0;

    #define BUILD(e, i) if (PyModule_AddObject(self, i, e)) {Py_XDECREF(e); Py_DECREF(self); return -1;}
    #define COLOR(n, r, g, b) BUILD(PyTuple_Pack(3, PyFloat_FromDouble(r), PyFloat_FromDouble(g), PyFloat_FromDouble(b)), n)
    #define PATH(e, i) BUILD(PyUnicode_FromString(constructFilepath(e)), i)

    BUILD(PyObject_CallFunctionObjArgs((PyObject *) &CursorType, NULL), "cursor")
    BUILD(PyObject_CallFunctionObjArgs((PyObject *) &KeyType, NULL), "key")
    BUILD(PyObject_CallFunctionObjArgs((PyObject *) &CameraType, NULL), "camera")
    BUILD(PyObject_CallFunctionObjArgs((PyObject *) &WindowType, NULL), "window")

    BUILD((PyObject *) &RectangleType, "Rectangle")
    BUILD((PyObject *) &ImageType, "Image")
    BUILD((PyObject *) &TextType, "Text")
    BUILD((PyObject *) &CircleType, "Circle")
    BUILD((PyObject *) &ShapeType, "Shape")
    BUILD((PyObject *) &PhysicsType, "Physics")

    BUILD(PyLong_FromLong(DYNAMIC), "DYNAMIC")
    BUILD(PyLong_FromLong(STATIC), "STATIC")

    PATH("images/man.png", "MAN")
    PATH("images/coin.png", "COIN")
    PATH("images/enemy.png", "ENEMY")
    PATH("images/plane.png", "PLANE")
    PATH("fonts/default.ttf", "DEFAULT")
    PATH("fonts/code.ttf", "CODE")
    PATH("fonts/pencil.ttf", "PENCIL")
    PATH("fonts/serif.ttf", "SERIF")
    PATH("fonts/handwriting.ttf", "HANDWRITING")
    PATH("fonts/typewriter.ttf", "TYPEWRITER")
    PATH("fonts/joined.ttf", "JOINED")

    COLOR("WHITE", 1, 1, 1)
    COLOR("BLACK", 0, 0, 0)
    COLOR("GRAY", .5, .5, .5)
    COLOR("DARK_GRAY", .2, .2, .2)
    COLOR("LIGHT_GRAY", .8, .8, .8)
    COLOR("BROWN", .6, .2, .2)
    COLOR("TAN", .8, .7, .6)
    COLOR("RED", 1, 0, 0)
    COLOR("DARK_RED", .6, 0, 0)
    COLOR("SALMON", 1, .5, .5)
    COLOR("ORANGE", 1, .5, 0)
    COLOR("GOLD", 1, .8, 0)
    COLOR("YELLOW", 1, 1, 0)
    COLOR("OLIVE", .5, .5, 0)
    COLOR("LIME", 0, 1, 0)
    COLOR("DARK_GREEN", 0, .4, 0)
    COLOR("GREEN", 0, .5, 0)
    COLOR("AQUA", 0, 1, 1)
    COLOR("BLUE", 0, 0, 1)
    COLOR("LIGHT_BLUE", .5, .8, 1)
    COLOR("AZURE", .9, 1, 1)
    COLOR("NAVY", 0, 0, .5)
    COLOR("PURPLE", .5, 0, 1)
    COLOR("PINK", 1, .75, .8)
    COLOR("MAGENTA", 1, 0, 1)

    #undef PATH
    #undef COLOR
    #undef BUILD

    uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    uint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

    const char *vertexSource =
        "#version 330 core\n"

        "in vec2 vertex;"
        "in vec2 coordinate;"
        "out vec2 position;"

        "uniform mat4 camera;"
        "uniform mat4 object;"

        "void main() {"
        "    gl_Position = camera * object * vec4(vertex, 0, 1);"
        "    position = coordinate;"
        "}";

    const char *fragmentSource =
        "#version 330 core\n"

        "in vec2 position;"
        "out vec4 fragment;"

        "uniform vec4 color;"
        "uniform sampler2D sampler;"
        "uniform int image;"

        "void main() {"
        "    if (image == " STR(TEXT) ") fragment = vec4(1, 1, 1, texture(sampler, position).r) * color;"
        "    else if (image == " STR(IMAGE) ") fragment = texture(sampler, position) * color;"
        "    else if (image == " STR(SHAPE) ") fragment = color;"
        "}";

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);

    uint buffer;
    float data[] = {-.5, .5, 0, 0, .5, .5, 1, 0, -.5, -.5, 0, 1, .5, -.5, 1, 1};

    glGenVertexArrays(1, &mesh);
    glBindVertexArray(mesh);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 64, data, GL_STATIC_DRAW);

    glVertexAttribPointer(glGetAttribLocation(program, "vertex"), 2, GL_FLOAT, GL_FALSE, 16, 0);
    glVertexAttribPointer(glGetAttribLocation(program, "coordinate"), 2, GL_FLOAT, GL_FALSE, 16, (void *) 8);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glDeleteBuffers(1, &buffer);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUniform1i(glGetUniformLocation(program, "sampler"), 0);

    return 0;
}

static void Module_free() {
    while (textures) {
        Texture *this = textures;
        glDeleteTextures(1, &this -> source);
        free(this -> name);

        textures = this -> next;
        free(this);
    }

    while (fonts) {
        Font *this = fonts;
        FT_Done_Face(this -> face);
        free(this -> name);

        fonts = this -> next;
        free(this);
    }

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &mesh);

    FT_Done_FreeType(library);
    glfwTerminate();

    Py_XDECREF(loop);
    Py_DECREF(path);
    Py_DECREF(window);
    Py_DECREF(cursor);
    Py_DECREF(key);
    Py_DECREF(camera);
}

static PyModuleDef_Slot ModuleSlots[] = {
    {Py_mod_exec, Module_exec},
    {0, NULL}
};

static struct PyModuleDef Module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "JoBase",
    .m_size = 0,
    .m_free = Module_free,
    .m_methods = ModuleMethods,
    .m_slots = ModuleSlots
};

PyMODINIT_FUNC PyInit_JoBase() {
    #define READY(t) if (PyType_Ready(&t)) return NULL;

    printf("Welcome to JoBase\n");
    srand(time(NULL));

    READY(VectorType)
    READY(ButtonType)
    READY(CursorType)
    READY(KeyType)
    READY(CameraType)
    READY(WindowType)
    READY(BaseType)
    READY(RectangleType)
    READY(ImageType)
    READY(TextType)
    READY(CircleType)
    READY(ShapeType)
    READY(PhysicsType)

    return PyModuleDef_Init(&Module);
    #undef READY
}