#define DEL(e) if(!e){return PyErr_SetString(PyExc_AttributeError,"can't delete attribute"),-1;}
#define SEQ(e) return format(PyExc_TypeError,"must be sequence, not %s",Py_TYPE(e)->tp_name),-1;
#define BASE(e, t) PyObject_IsInstance(e,(PyObject*)&t)

#define NEW(t, e) t*i=malloc(sizeof(t));i->next=e;e=i;
#define ERR(e) ((e)==-1&&PyErr_Occurred())
#define FOR(t, e) for(t i=0;i<e;i++)
#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)
#define IDX(e) (e-2)*3

#define _USE_MATH_DEFINES
#define SHAPE 0
#define IMAGE 1
#define TEXT 2

#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

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
typedef double (*Getter)(PyObject *, uint8_t);
typedef GLfloat mat[9];
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
    vec2 pos;
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
    vec3 color;
    vec2 size;
    char *caption;
    bool resize;
} Window;

typedef struct Base {
    PyObject_HEAD
    vec2 vel;
    vec2 pos;
    vec2 scale;
    vec2 anchor;
    vec4 color;
    double elasticity;
    double friction;
    size_t length;
    cpShape **shapes;
    cpBody *body;
    bool rotate;
    cpFloat (*moment)(struct Base *);
    void (*new)(struct Base *);
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
    size_t vertex;
    poly points;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
} Shape;

typedef struct Line {
    Shape shape;
    double width;
} Line;

typedef struct Joint {
    PyObject_HEAD
    double width;
    cpConstraint *joint;
    vec4 color;
    Base *a;
    Base *b;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
} Joint;

typedef struct Physics {
    PyObject_HEAD
    cpSpace *space;
    PyObject **data;
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
extern PyTypeObject LineType;
extern PyTypeObject ShapeType;
extern PyTypeObject PhysicsType;
extern PyTypeObject JointType;
extern PyTypeObject PinType;
extern PyTypeObject PivotType;
extern PyTypeObject MotorType;
extern PyTypeObject SpringType;
extern PyTypeObject GrooveType;

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
extern GLuint program;
extern GLuint mesh;
extern GLint uniform[7];

// extern void start();
// extern void end();
extern void parameters();
extern void rectangleDraw(Rectangle *, uint8_t);
extern void rectanglePoly(Rectangle *, poly);
extern void rotate(poly, size_t, double, vec2);
extern void format(PyObject *, const char *, ...);
extern void lineCreate(poly, size_t, double);
extern void jointDraw(Joint *, poly, size_t);
extern void buffers(GLuint *, GLuint *, GLuint *);
extern void baseMatrix(Base *, double, double);
extern void baseStart(Base *, double);
extern void baseUniform(mat, vec4);
extern void baseMoment(Base *);
extern void baseInit(Base *);
extern void baseDealloc(Base *);
extern void shapeDealloc(Shape *);
extern void jointDealloc(Joint *);
extern void jointInit(Joint *);

extern const char *filepath(const char *);
extern Base *shapeNew(PyTypeObject *);
extern Button *buttonNew(Set *);
extern Vector *vectorNew(PyObject *, Getter, uint8_t);
extern PyObject *collide(PyObject *, PyObject *);
extern PyObject *shapeDraw(Shape *, PyObject *);
extern PyObject *rectangleNew(PyTypeObject *, PyObject *, PyObject *);
extern PyObject *baseNew(PyTypeObject *, size_t);
extern PyObject *jointNew(PyTypeObject *, cpConstraint *);
extern poly shapePoly(Shape *);

// extern vec cursorPos();
// extern vec windowSize();

extern int baseToward(vec2, PyObject *);
extern int baseSmooth(vec2, PyObject *);
extern int vectorSet(PyObject *, vec, uint8_t);
extern int shapeParse(Shape *, PyObject *);
extern int jointStart(Joint *, PyObject *);
extern int update();

extern double shapeLeft(Shape *);
extern double shapeTop(Shape *);
extern double shapeRight(Shape *);
extern double shapeBottom(Shape *);
extern double getLeft(poly, size_t);
extern double getTop(poly, size_t);
extern double getRight(poly, size_t);
extern double getBottom(poly, size_t);
extern double circleX(Circle *);
extern double circleY(Circle *);

extern int jsWidth();
extern int jsHeight();
extern int jsWait();
extern void jsStart();
extern void jsEnd();