#define DEL(e) if(!e){return PyErr_SetString(PyExc_AttributeError,"can't delete attribute"),-1;}
#define SEQ(e) return format(PyExc_TypeError,"must be sequence, not %s",Py_TYPE(e)->tp_name),-1;

#define NEW(t, e) t*i=malloc(sizeof(t));i->next=e;e=i;
#define ERR(e) ((e)==-1&&PyErr_Occurred())
#define FOR(t, e) for(t i=0;i<e;i++)
#define AVR(e) (e[x]+e[y])/2
#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)

#define DYNAMIC CP_BODY_TYPE_DYNAMIC
#define STATIC CP_BODY_TYPE_STATIC

#define SHAPE 0
#define IMAGE 1
#define TEXT 2

#include <chipmunk/chipmunk.h>
#include <GLFW/glfw3.h>
#include <Python.h>
#include <ft2build.h>
#include <stdbool.h>
#include FT_FREETYPE_H

enum {x, y, z};
enum {r, g, b, a};
enum {vert, coord, view, obj, color, img};

typedef double *vec;
typedef double vec2[2];
typedef double vec3[3];
typedef double vec4[4];
typedef vec (*Getter)(PyObject *);
typedef GLfloat mat[16];
typedef vec2 *poly;

typedef struct Item {
    const char *name;
    setter set;
} Item;

typedef struct Vector {
    PyObject_HEAD
    PyObject *parent;
    Getter get;
    uint8_t size;
    Item data[4];
} Vector;

typedef struct Set {
    const char *key;
    bool hold;
    bool press;
    bool release;
    bool repeat;
} Set;

typedef struct Button {
    PyObject_HEAD
    Set *state;
} Button;

typedef struct Cursor {
    PyObject_HEAD
    Set buttons[GLFW_MOUSE_BUTTON_LAST + 1];
    bool move;
    bool enter;
    bool leave;
    bool press;
    bool release;
} Cursor;

typedef struct Key {
    PyObject_HEAD
    Set keys[GLFW_KEY_LAST + 1];
    bool press;
    bool release;
    bool repeat;
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
    bool resize;
} Window;

typedef struct Base {
    PyObject_HEAD
    vec2 vel;
    vec2 pos;
    vec2 scale;
    vec2 anchor;
    vec4 color;
    double angle;
    double mass;
    double elasticity;
    double friction;
    double angular;
    int type;
    cpShape *shape;
    cpBody *body;
    bool rotate;
    cpFloat (*moment)(struct Base *);
    void (*new)(struct Base *);
    void (*base)(struct Base *);
    double (*top)(struct Base *);
    double (*bottom)(struct Base *);
    double (*left)(struct Base *);
    double (*right)(struct Base *);
} Base;

typedef struct Rectangle {
    Base base;
    vec2 size;
} Rectangle;

typedef struct Texture {
    struct Texture *next;
    GLuint src;
    FT_Vector size;
    char *name;
} Texture;

typedef struct Image {
    Rectangle rect;
    Texture *texture;
} Image;

typedef struct Char {
    FT_Pos advance;
    FT_Vector size;
    FT_Vector pos;
    bool load;
    GLuint src;
    int font;
} Char;

typedef struct Font {
    struct Font *next;
    FT_Face face;
    char *name;
} Font;

typedef struct Text {
    Rectangle rect;
    wchar_t *content;
    Char *chars;
    Font *font;
    FT_Vector base;
    FT_Pos descend;
    double size;
} Text;

typedef struct Circle {
    Base base;
    double radius;
    GLuint vao;
    GLuint vbo;
} Circle;

typedef struct Shape {
    Base base;
    poly points;
    size_t *indices;
    size_t vertex;
    size_t index;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
} Shape;

typedef struct Physics {
    PyObject_HEAD
    cpSpace *space;
    Base **data;
    size_t length;
} Physics;

extern PyTypeObject VectorType;
extern PyTypeObject ButtonType;
extern PyTypeObject CursorType;
extern PyTypeObject KeyType;
extern PyTypeObject CameraType;
extern PyTypeObject WindowType;
extern PyTypeObject BaseType;
extern PyTypeObject RectangleType;
extern PyTypeObject ImageType;
extern PyTypeObject TextType;
extern PyTypeObject CircleType;
extern PyTypeObject ShapeType;
extern PyTypeObject PhysicsType;

extern Window *window;
extern Cursor *cursor;
extern Camera *camera;
extern Key *key;

extern FT_Library library;
extern PyObject *loop;
extern Texture *textures;
extern Font *fonts;

extern char *path;
extern size_t length;
extern bool ready;
extern GLuint program;
extern GLuint mesh;
extern GLint uniform[7];

extern void start();
extern void end();
extern void parameters();
extern void rectangleDraw(Rectangle *, uint8_t);
extern void polyRect(Rectangle *, poly);
extern void format(PyObject *, const char *, ...);
extern void baseMatrix(Base *, double, double);
extern void baseUniform(mat, vec4);
extern void baseMoment(Base *);
extern void baseInit(Base *);

extern const char *filepath(const char *);
extern Button *buttonNew(Set *);
extern poly polyShape(Shape *);
extern Vector *vectorNew(PyObject *, Getter, uint8_t);
extern PyObject *collide(PyObject *, PyObject *);
extern PyObject *rectangleNew(PyTypeObject *);

extern vec cursorPos();
extern vec windowSize();

extern int baseToward(vec2, PyObject *);
extern int baseSmooth(vec2, PyObject *);
extern int vectorSet(PyObject *, vec, uint8_t);
extern int update();

extern double polyLeft(poly, size_t);
extern double polyTop(poly, size_t);
extern double polyRight(poly, size_t);
extern double polyBottom(poly, size_t);
extern double circleX(Circle *);
extern double circleY(Circle *);