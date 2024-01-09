#define DEL(e, n) if(!e)return PyErr_SetString(PyExc_AttributeError,"cannot delete the "n" attribute"),-1;
#define END(e) return PyErr_Format(PyExc_TypeError,"%s must be derived from Base or Cursor",Py_TYPE(e)->tp_name),NULL;
#define REM(t, l, e) if (l==e)l=l->next;else for(t*i=l;i;i=i->next)if(i->next==e){i->next=i->next->next;break;}
#define NEW(t, e) t*i=malloc(sizeof(t));i->next=e;e=i;
#define ERR(e) ((e)==-1&&PyErr_Occurred())
#define INIT(e) if(e)return -1;
#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)

#define _USE_MATH_DEFINES
#define PY_SSIZE_T_CLEAN

#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES
#define VERSION "300 es"
#include <emscripten.h>
#else
#define VERSION "330 core"
#include <glad/glad.h>
#endif

#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_unsafe.h>
#include <GLFW/glfw3.h>
#include <Python.h>
#include <ft2build.h>
#include <stdbool.h>
#include FT_FREETYPE_H

enum {x, y, z};
enum {r, g, b, a};
enum {shape, image, text};
enum {vert, view, obj, color, img};

typedef struct Vec2 Vec2;
typedef struct Vec3 Vec3;
typedef struct Vec4 Vec4;
typedef struct Sides Sides;
typedef struct Set Set;
typedef struct Button Button;
typedef struct Array Array;
typedef struct Texture Texture;
typedef struct Font Font;
typedef struct Char Char;
typedef struct Vector Vector;
typedef struct Window Window;
typedef struct Cursor Cursor;
typedef struct Camera Camera;
typedef struct Key Key;
typedef struct Base Base;
typedef struct Physics Physics;
typedef struct Group Group;
typedef struct Body Body;
typedef struct Rectangle Rectangle;
typedef struct Image Image;
typedef struct Text Text;
typedef struct Circle Circle;
typedef struct Shape Shape;
typedef struct Line Line;
typedef struct Points Points;
typedef struct Joint Joint;
typedef struct Pin Pin;
typedef struct Spring Spring;
typedef struct Groove Groove;
// typedef struct Pivot Pivot;

typedef double *vec;
typedef void (*joint)(Joint *);
typedef void (*move)(Joint *);
typedef int (*set)(PyObject *);
typedef int (*reset)(Shape *);

struct Vec2 {
    double x;
    double y;
};

struct Vec3 {
    double r;
    double g;
    double b;
};

struct Vec4 {
    double r;
    double g;
    double b;
    double a;
};

struct Sides {
    double top;
    double bottom;
    double left;
    double right;
};

struct Set {
    const char *name;
    bool hold;
    bool press;
    bool release;
    bool repeat;
};

struct Button {
    PyObject_HEAD
    Set *key;
};

struct Array {
    Array *next;
    Joint *src;
};

struct Texture {
    Texture *next;
    char *name;
    Vec2 size;
    GLuint src;
};

struct Font {
    Font *next;
    char *name;
    FT_Face face;
};

struct Char {
    GLuint src;
    int advance;
    Vec2 size;
    Vec2 pos;
    double font;
};

struct Vector {
    PyObject_HEAD
    PyObject *parent;
    char names[4];
    uint8_t size;
    vec vect;
    set set;
};

struct Window {
    PyObject_HEAD
    GLFWwindow *glfw;
    bool resize;
    char *title;
    Vec2 size;
    Vec3 color;
};

struct Cursor {
    PyObject_HEAD
    Set buttons[GLFW_MOUSE_BUTTON_LAST + 1];
    Vec2 pos;
    bool move;
    bool enter;
    bool leave;
    bool press;
    bool release;
};

struct Camera {
    PyObject_HEAD
    Vec2 pos;
    Vec2 scale;
};

struct Key {
    PyObject_HEAD
    Set keys[GLFW_KEY_LAST + 1];
    bool press;
    bool release;
    bool repeat;
};

struct Base {
    PyObject_HEAD
    Vec4 color;
    Vec2 pos;
    Vec2 scale;
    Vec2 anchor;
    Vec2 transform;
    double angle;
    double rotate;
    double mass;
    double elasticity;
    double friction;
    Body *body;
    Base *next;
    cpShape *shape;
    Array *joint;
    Group *group;
    cpShape *(*physics)(Base *);
    Sides (*sides)(Base *);
    void (*unsafe)(Base *);
};

struct Physics {
    PyObject_HEAD
    cpSpace *space;
    Body *list;
    Vec2 gravity;
    int group;
};

struct Body {
    PyObject_HEAD
    Physics *parent;
    Vec2 velocity;
    Base *list;
    cpBody *body;
    Body *next;
};

struct Rectangle {
    Base base;
    Vec2 size;
};

struct Image {
    Rectangle base;
    Texture *src;
};

struct Text {
    Rectangle base;
    wchar_t *content;
    Char *chars;
    Font *src;
    Vec2 vect;
    int descend;
    double size;
};

struct Circle {
    Base base;
    double diameter;
    GLuint vao;
    GLuint vbo;
};

struct Shape {
    Base base;
    size_t length;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint *indices;
    Vec2 *points;
    reset reset;
};

struct Line {
    Shape base;
    double width;
};

struct Points {
    PyObject_HEAD
    Shape *parent;
    reset method;
};

struct Group {
    PyObject_HEAD
    int id;
};

struct Joint {
    PyObject_HEAD
    double width;
    Physics *parent;
    joint create;
    move unsafe;
    cpConstraint *joint;
    Vec4 color;
    Base *a;
    Base *b;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
};

struct Pin {
    Joint base;
    Vec2 start;
    Vec2 end;
    double length;
};

struct Spring {
    Joint base;
    Vec2 start;
    Vec2 end;
    double length;
    double stiffness;
    double damping;
};

struct Groove {
    Joint base;
    Vec2 start;
    Vec2 end;
    Vec2 groove;
};

// struct Pivot {
//     Joint base;
//     Vec2 start;
//     Vec2 end;
// };

extern PyTypeObject PointsType;
extern PyTypeObject VectorType;
extern PyTypeObject ButtonType;
extern PyTypeObject WindowType;
extern PyTypeObject CursorType;
extern PyTypeObject CameraType;
extern PyTypeObject KeyType;
extern PyTypeObject BaseType;
extern PyTypeObject RectangleType;
extern PyTypeObject ImageType;
extern PyTypeObject TextType;
extern PyTypeObject CircleType;
extern PyTypeObject ShapeType;
extern PyTypeObject LineType;
extern PyTypeObject PhysicsType;
extern PyTypeObject GroupType;
extern PyTypeObject BodyType;
extern PyTypeObject JointType;
extern PyTypeObject PinType;
extern PyTypeObject SpringType;
extern PyTypeObject GrooveType;
// extern PyTypeObject PivotType;

extern FT_Library library;
extern Texture *textures;
extern Font *fonts;

extern GLint uniforms[];
extern GLuint program;
extern GLuint mesh;

extern PyObject *module;
extern Window *window;
extern Cursor *cursor;
extern Camera *camera;
extern Key *key;

extern Vector *Vector_new(PyObject *, vec, uint8_t, set);
extern Shape *Shape_new(PyTypeObject *, PyObject *, PyObject *);
extern Rectangle *Rectangle_new(PyTypeObject *, PyObject *, PyObject *);
extern Points *Points_new(Shape *, reset);
extern Joint *Joint_new(PyTypeObject *, joint, move, cpConstraint *);
extern PyObject *Base_collide(PyObject *, PyObject *);
extern PyObject *Shape_render(Shape *, size_t);
extern Button *Button_new(Set *);
extern Body *Body_new(Physics *, PyObject *);
extern Sides Base_sides(Base *, Vec2 *, size_t);
extern Sides Shape_sides(Shape *);
extern cpTransform Base_transform(Base *);
extern Vec2 Circle_pos(Circle *);
extern cpVect Joint_rotate(Base *, Vec2);
extern cpVect Body_set(Body *, Vec2);
extern Vec2 Body_get(Body *, cpVect);
extern Group *Group_new(int);

extern int Vector_set(PyObject *, vec, uint8_t);
extern int Points_set(Shape *, PyObject *);
extern double Base_radius(Base *, double);
extern bool Joint_active(Joint *);

extern void Joint_unsafe(Joint *);
extern void Line_create(Vec2 *, size_t, double);
extern void Joint_draw(Joint *, Vec2 *, size_t);
extern void Base_uniform(GLfloat *, Vec4, uint8_t);
extern void Base_matrix(Base *, uint8_t, double, double);
extern void Base_poly(Base *, Vec2 *, Vec2 *, size_t);
extern void Rectangle_render(Rectangle *, uint8_t);
extern void Rectangle_poly(Rectangle *, Vec2 *);
extern void Shape_poly(Shape *, Vec2 *);
extern void Shape_dealloc(Shape *);
extern void Base_unsafe(Base *);
extern void Shape_reduce(Shape *, cpShape *);
extern void Base_shape(Base *, cpShape *);
extern void Base_buffers(GLuint *, GLuint *, GLuint *);
extern void Base_clean(Base *);
extern void Joint_add(Joint *);
extern void Joint_check(Joint *);

extern int width();
extern int height();