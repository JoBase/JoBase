#define PY_SSIZE_T_CLEAN
#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES

#define CHECK(value) if (checkValue(value)) return -1

#include <Python.h>
#include <structmember.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image/stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef _WIN32
PyMODINIT_FUNC PyInit___init__;
#endif

typedef struct Set {
    const char *key;
    unsigned char hold;
    unsigned char press;
    unsigned char release;
    unsigned char repeat;
} Set;

typedef struct Vec2 {
    double x;
    double y;
} Vec2;

typedef struct Vec4 {
    double x;
    double y;
    double z;
    double w;
} Vec4;

typedef struct Texture {
    const char *name;
    Vec2 size;
    GLuint source;
    struct Texture *next;
} Texture;

typedef struct Vector {
    PyObject_HEAD
    PyObject *parent;
    getter getX;
    setter setX;
    getter getY;
    setter setY;
    getter getZ;
    setter setZ;
    getter getW;
    setter setW;
    reprfunc repr;
    reprfunc str;
} Vector;

typedef struct Cursor {
    PyObject_HEAD
    Vec2 pos;
    unsigned char move;
    unsigned char enter;
    unsigned char leave;
    unsigned char press;
    unsigned char release;
} Cursor;

typedef struct Key {
    PyObject_HEAD
    Set keys[GLFW_KEY_LAST + 1];
    unsigned char press;
    unsigned char release;
    unsigned char repeat;
} Key;

typedef struct Camera {
    PyObject_HEAD
    Vec2 pos;
    Vec2 view;
} Camera;

typedef struct Window {
    PyObject_HEAD
    GLFWwindow *window;
    const char *caption;
    Vec4 color;
    Vec2 size;
    unsigned char resize;
} Window;

typedef struct Shape {
    PyObject_HEAD
    Vec2 pos;
    Vec2 scale;
    Vec2 anchor;
    Vec4 color;
    double angle;
} Shape;

typedef struct Rectangle {
    Shape shape;
    Vec2 size;
} Rectangle;

typedef struct Image {
    Rectangle rect;
    Texture *texture;
} Image;

static Texture *textures;
static PyObject *error;
static Cursor *cursor;
static Key *key;
static Camera *camera;
static Window *window;

static PyTypeObject VectorType;
static PyTypeObject ShapeType;

static char *path;
static unsigned short length;
static GLuint program;
static GLuint mesh;

static const char *pathAdd(const char *file) {
    path[length] = 0;
    return strcat(path, file);
}

static int checkValue(PyObject *value) {
    if (!value) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete attribute");
        return 1;
    }

    return 0;
}

static void windowSizeCallback(GLFWwindow *Py_UNUSED(window), int width, int height) {
    window -> size.x = width;
    window -> size.y = height;
    window -> resize = 1;
}

static void framebufferSizeCallback(GLFWwindow *Py_UNUSED(window), int width, int height) {
    glViewport(0, 0, width, height);
}

static void cursorPosCallback(GLFWwindow *Py_UNUSED(window), double x, double y) {
    cursor -> pos.x = x;
    cursor -> pos.y = y;
    cursor -> move = 1;
}

static void cursorEnterCallback(GLFWwindow *Py_UNUSED(window), int entered) {
    if (entered) cursor -> enter = 1;
    else cursor -> leave = 1;
}

static void mouseButtonCallback(GLFWwindow *Py_UNUSED(window), int Py_UNUSED(button), int action, int Py_UNUSED(mods)) {
    if (action == GLFW_PRESS)
        cursor -> press = 1;

    else if (action == GLFW_RELEASE)
        cursor -> release = 1;
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

static void memoryCleanup() {
    while (textures) {
        Texture *this = textures;
        glDeleteTextures(1, &this -> source);

        textures = this -> next;
        free(this);
    }

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &mesh);
    glfwTerminate();

    Py_DECREF(path);
    Py_DECREF(error);
    Py_DECREF(window);
    Py_DECREF(cursor);
    Py_DECREF(key);
    Py_DECREF(camera);
}

static double constrainValue(double value, double min, double max) {
    return value > max ? max : value < min ? min : value;
}

static unsigned char collideLineLine(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4) {
    double value = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
    double u1 = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / value;
    double u2 = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / value;

    return u1 >= 0 && u1 <= 1 && u2 >= 0 && u2 <= 1;
}

static unsigned char collidePolyLine(Vec2 *poly, unsigned char size, Vec2 p1, Vec2 p2) {
    for (unsigned char i = 0; i < size; i ++) {
        Vec2 p3 = poly[i];
        Vec2 p4 = poly[i + 1 == size ? 0 : i + 1];

        if (collideLineLine(p1, p2, p3, p4))
            return 1;
    }

    return 0;
}

static unsigned char collidePolyPoint(Vec2 *poly, unsigned char size, Vec2 point) {
    unsigned char hit = 0;

    for (unsigned char i = 0; i < size; i ++) {
        Vec2 v1 = poly[i];
        Vec2 v2 = poly[i + 1 == size ? 0 : i + 1];

        if (((v1.y > point.y && v2.y < point.y) || (v1.y < point.y && v2.y > point.y)) &&
            (point.x < (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x))
                hit = !hit;
    }

    return hit;
}

static unsigned char collidePolyPoly(Vec2 *p1, unsigned char s1, Vec2 *p2, unsigned char s2) {
    if (collidePolyPoint(p1, s1, p2[0]) || collidePolyPoint(p2, s2, p1[0]))
        return 1;

    for (unsigned char i = 0; i < s1; i ++) {
        Vec2 v1 = p1[i];
        Vec2 v2 = p1[i + 1 == s1 ? 0 : i + 1];

        if (collidePolyLine(p2, s2, v1, v2))
            return 1;
    }

    return 0;
}

static void posPoly(Vec2 *poly, unsigned char size, Vec2 pos) {
    for (unsigned char i = 0; i < size; i ++) {
        poly[i].x += pos.x;
        poly[i].y += pos.y;
    }
}

static void scalePoly(Vec2 *poly, unsigned char size, Vec2 scale) {
    for (unsigned char i = 0; i < size; i ++) {
        poly[i].x *= scale.x;
        poly[i].y *= scale.y;
    }
}

static void rotPoly(Vec2 *poly, unsigned char size, double angle) {
    double cosine = cos(angle * M_PI / 180);
    double sine = sin(angle * M_PI / 180);

    for (unsigned char i = 0; i < size; i ++) {
        double x = poly[i].x;
        double y = poly[i].y;

        poly[i].x = x * cosine - y * sine;
        poly[i].y = x * sine + y * cosine;
    }
}

static double getPolyLeft(Vec2 *poly, unsigned char size) {
    double left = poly[0].x;

    for (unsigned char i = 1; i < size; i ++)
        if (poly[i].x < left)
            left = poly[i].x;

    return left;
}

static double getPolyTop(Vec2 *poly, unsigned char size) {
    double top = poly[0].y;

    for (unsigned char i = 1; i < size; i ++)
        if (poly[i].y > top)
            top = poly[i].y;

    return top;
}

static double getPolyRight(Vec2 *poly, unsigned char size) {
    double right = poly[0].x;

    for (unsigned char i = 1; i < size; i ++)
        if (poly[i].x > right)
            right = poly[i].x;

    return right;
}

static double getPolyBottom(Vec2 *poly, unsigned char size) {
    double bottom = poly[0].y;

    for (unsigned char i = 1; i < size; i ++)
        if (poly[i].y < bottom)
            bottom = poly[i].y;

    return bottom;
}

static void getRectanglePoly(Rectangle *rect, Vec2 *poly) {
    Vec2 data[] = {{-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5}};
    Vec2 size = {rect -> size.x * rect -> shape.scale.x, rect -> size.y * rect -> shape.scale.y};
    
    scalePoly(data, 4, size);
    posPoly(data, 4, rect -> shape.anchor);
    rotPoly(data, 4, rect -> shape.angle);
    posPoly(data, 4, rect -> shape.pos);

    for (unsigned char i = 0; i < 4; i ++)
        poly[i] = data[i];
}

static PyObject *checkShapesCollide(Vec2 *poly, unsigned char size, PyObject *shape) {
    Vec2 points[4];

    getRectanglePoly((Rectangle *) shape, points);
    return PyBool_FromLong(collidePolyPoly(poly, size, points, 4));
}

static GLfloat switchValues(GLfloat *matrix, unsigned char i, unsigned char j) {
    unsigned char value = 2 + (j - i);

    i += 4 + value;
    j += 4 - value;

    #define e(a, b) matrix[((j + b) % 4) * 4 + ((i + a) % 4)]

    GLfloat final =
        e(1, -1) * e(0, 0) * e(-1, 1) +
        e(1, 1) * e(0, -1) * e(-1, 0) +
        e(-1, -1) * e(1, 0) * e(0, 1) -
        e(-1, -1) * e(0, 0) * e(1, 1) -
        e(-1, 1) * e(0, -1) * e(1, 0) -
        e(1, -1) * e(-1, 0) * e(0, 1);

    return value % 2 ? final : -final;
    #undef e
}

static PyObject *strPos(Vec2 pos) {
    char buffer[29];
    sprintf(buffer, "(%g, %g)", pos.x, pos.y);

    return PyUnicode_FromString(buffer);
}

static PyObject *reprPos(Vec2 pos) {
    char buffer[29];
    sprintf(buffer, "[%g, %g]", pos.x, pos.y);

    return PyUnicode_FromString(buffer);
}

static int setPos(PyObject *value, Vec2 *pos) {
    CHECK(value);
    
    PyObject *x = NULL;
    PyObject *y = NULL;

    if (Py_TYPE(value) == &VectorType) {
        Vector *object = (Vector *) value;

        if (object -> getX)
            x = object -> getX(object -> parent, NULL);

        if (object -> getY)
            y = object -> getY(object -> parent, NULL);
    }

    else if (PyTuple_Check(value)) {
        size_t size = PyTuple_GET_SIZE(value);

        if (size < 2) {
            PyErr_SetString(PyExc_IndexError, "The position attribute must contain at least three values");
            return -1;
        }

        x = PyTuple_GET_ITEM(value, 0);
        y = PyTuple_GET_ITEM(value, 1);
    }

    else if (PyList_Check(value)) {
        size_t size = PyList_GET_SIZE(value);

        if (size < 2) {
            PyErr_SetString(PyExc_IndexError, "The position attribute must contain at least three values");
            return -1;
        }

        x = PyTuple_GET_ITEM(value, 0);
        y = PyTuple_GET_ITEM(value, 1);
    }

    else if (PyDict_Check(value)) {
        x = PyDict_GetItemString(value, "x");

        if (!x) {
            PyErr_SetString(PyExc_AttributeError, "The position attribute must contain a x variable");
            return -1;
        }

        y = PyDict_GetItemString(value, "y");

        if (!y) {
            PyErr_SetString(PyExc_AttributeError, "The position attribute must contain a y variable");
            return -1;
        }
    }

    else {
        PyErr_SetString(PyExc_TypeError, "The position attribute must be a list, dictionary or tuple");
        return -1;
    }

    if ((pos -> x = PyFloat_AsDouble(x)) < 0 && PyErr_Occurred())
        return -1;

    if ((pos -> y = PyFloat_AsDouble(y)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *strColor(Vec4 color) {
    char buffer[57];
    sprintf(buffer, "(%g, %g, %g, %g)", color.x, color.y, color.z, color.w);

    return PyUnicode_FromString(buffer);
}

static PyObject *reprColor(Vec4 color) {
    char buffer[57];
    sprintf(buffer, "[%g, %g, %g, %g]", color.x, color.y, color.z, color.w);

    return PyUnicode_FromString(buffer);
}

static int setColor(PyObject *value, Vec4 *color) {
    CHECK(value);

    PyObject *red = NULL;
    PyObject *green = NULL;
    PyObject *blue = NULL;
    PyObject *alpha = NULL;

    if (Py_TYPE(value) == &VectorType) {
        Vector *object = (Vector *) value;

        if (object -> getX)
            red = object -> getX(object -> parent, NULL);

        if (object -> getY)
            green = object -> getY(object -> parent, NULL);

        if (object -> getZ)
            blue = object -> getZ(object -> parent, NULL);

        if (object -> getW)
            alpha = object -> getW(object -> parent, NULL);
    }

    else if (PyTuple_Check(value)) {
        size_t size = PyTuple_GET_SIZE(value);

        if (size < 3) {
            PyErr_SetString(PyExc_IndexError, "The color attribute must contain at least three values");
            return -1;
        }

        red = PyTuple_GET_ITEM(value, 0);
        green = PyTuple_GET_ITEM(value, 1);
        blue = PyTuple_GET_ITEM(value, 2);

        if (size > 3)
            alpha = PyTuple_GET_ITEM(value, 3);
    }

    else if (PyList_Check(value)) {
        size_t size = PyList_GET_SIZE(value);

        if (size < 3) {
            PyErr_SetString(PyExc_IndexError, "The color attribute must contain at least three values");
            return -1;
        }

        red = PyList_GET_ITEM(value, 0);
        green = PyList_GET_ITEM(value, 1);
        blue = PyList_GET_ITEM(value, 2);

        if (size > 3)
            alpha = PyList_GET_ITEM(value, 3);
    }

    else if (PyDict_Check(value)) {
        red = PyDict_GetItemString(value, "red");

        if (!red) {
            PyErr_SetString(PyExc_AttributeError, "The color attribute must contain a red variable");
            return -1;
        }

        green = PyDict_GetItemString(value, "green");

        if (!green) {
            PyErr_SetString(PyExc_AttributeError, "The color attribute must contain a green variable");
            return -1;
        }

        blue = PyDict_GetItemString(value, "blue");

        if (!blue) {
            PyErr_SetString(PyExc_AttributeError, "The color attribute must contain a blue variable");
            return -1;
        }

        alpha = PyDict_GetItemString(value, "alpha");
    }

    else {
        PyErr_SetString(PyExc_TypeError, "The color attribute must be a list, dictionary or tuple");
        return -1;
    }

    if ((color -> x = PyFloat_AsDouble(red)) < 0 && PyErr_Occurred())
        return -1;

    if ((color -> y = PyFloat_AsDouble(green)) < 0 && PyErr_Occurred())
        return -1;

    if ((color -> z = PyFloat_AsDouble(blue)) < 0 && PyErr_Occurred())
        return -1;

    if (alpha && (color -> w = PyFloat_AsDouble(alpha)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static void newMatrix(GLfloat *matrix) {
    for (unsigned char i = 0; i < 16; i ++)
        matrix[i] = i % 5 ? 0 : 1;
}

static void invMatrix(GLfloat *matrix) {
    GLfloat result[16];
    GLfloat value = 0;

    for (unsigned char i = 0; i < 4; i ++)
        for (unsigned char j = 0; j < 4; j ++)
            result[j * 4 + i] = switchValues(matrix, i, j);

    for (unsigned char i = 0; i < 4; i ++)
        value += matrix[i] * result[i * 4];

    for (unsigned char i = 0; i < 16; i ++)
        matrix[i] = result[i] * value;
}

static void mulMatrix(GLfloat *a, GLfloat *b) {
    GLfloat result[16];

    for (unsigned char i = 0; i < 16; i ++) {
        unsigned char x = i - i / 4 * 4;
        unsigned char y = i / 4 * 4;

        result[i] =
            a[y] * b[x] + a[y + 1] * b[x + 4] +
            a[y + 2] * b[x + 8] + a[y + 3] * b[x + 12];
    }

    for (unsigned char i = 0; i < 16; i ++)
        a[i] = result[i];
}

static void posMatrix(GLfloat *matrix, Vec2 pos) {
    GLfloat base[16];
    newMatrix(base);

    base[12] = (GLfloat) pos.x;
    base[13] = (GLfloat) pos.y;
    mulMatrix(matrix, base);
}

static void scaleMatrix(GLfloat *matrix, Vec2 scale) {
    GLfloat base[16];
    newMatrix(base);

    base[0] = (GLfloat) scale.x;
    base[5] = (GLfloat) scale.y;
    mulMatrix(matrix, base);
}

static void rotMatrix(GLfloat *matrix, double angle) {
    GLfloat base[16];
    newMatrix(base);

    double sine = sin(angle * M_PI / 180);
	double cosine = cos(angle * M_PI / 180);

    base[0] = (GLfloat) cosine;
    base[1] = (GLfloat) sine;
    base[4] = (GLfloat) -sine;
    base[5] = (GLfloat) cosine;
    mulMatrix(matrix, base);
}

static void viewMatrix(GLfloat *matrix, Vec2 view) {
    GLfloat base[16];
    newMatrix(base);

    base[0] = 2 / (GLfloat) window -> size.x;
    base[5] = 2 / (GLfloat) window -> size.y;
    base[10] = -2 / (GLfloat) (view.y - view.x);
    base[14] = (GLfloat) ((-view.y + view.x) / (view.y - view.x));
    mulMatrix(matrix, base);
}

static void updateView() {
    GLfloat matrix[16];

    newMatrix(matrix);
    posMatrix(matrix, camera -> pos);
    invMatrix(matrix);
    viewMatrix(matrix, camera -> view);
    
    glUniformMatrix4fv(
        glGetUniformLocation(program, "camera"),
        1, GL_FALSE, matrix);
}

static void drawRectangle(Rectangle *rect) {
    Vec2 size = {rect -> size.x * rect -> shape.scale.x, rect -> size.y * rect -> shape.scale.y};
    GLfloat matrix[16];

    newMatrix(matrix);
    scaleMatrix(matrix, size);
    posMatrix(matrix, rect -> shape.anchor);
    rotMatrix(matrix, rect -> shape.angle);
    posMatrix(matrix, rect -> shape.pos);

    glUniformMatrix4fv(
        glGetUniformLocation(program, "object"),
        1, GL_FALSE, matrix);

    glUniform4f(
        glGetUniformLocation(program, "color"), (GLfloat) rect -> shape.color.x,
        (GLfloat) rect -> shape.color.y, (GLfloat) rect -> shape.color.z,
        (GLfloat) rect -> shape.color.w);

    glBindVertexArray(mesh);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

static PyObject *Vector_getRed(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getX) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the red attribute");
        return NULL;
    }

    return self -> getX(self -> parent, NULL);
}

static int Vector_setRed(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setX) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the red attribute");
        return -1;
    }

    return self -> setX(self -> parent, value, NULL);
}

static PyObject *Vector_getGreen(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getY) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the green attribute");
        return NULL;
    }

    return self -> getY(self -> parent, NULL);
}

static int Vector_setGreen(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setY) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the green attribute");
        return -1;
    }

    return self -> setY(self -> parent, value, NULL);
}

static PyObject *Vector_getBlue(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getZ) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the blue attribute");
        return NULL;
    }

    return self -> getZ(self -> parent, NULL);
}

static int Vector_setBlue(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setZ) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the blue attribute");
        return -1;
    }

    return self -> setZ(self -> parent, value, NULL);
}

static PyObject *Vector_getAlpha(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getW) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the alpha attribute");
        return NULL;
    }

    return self -> getW(self -> parent, NULL);
}

static int Vector_setAlpha(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setW) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the alpha attribute");
        return -1;
    }

    return self -> setW(self -> parent, value, NULL);
}

static PyObject *Vector_getX(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getX) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the x attribute");
        return NULL;
    }

    return self -> getX(self -> parent, NULL);
}

static int Vector_setX(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setX) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the x attribute");
        return -1;
    }

    return self -> setX(self -> parent, value, NULL);
}

static PyObject *Vector_getY(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getY) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the y attribute");
        return NULL;
    }

    return self -> getY(self -> parent, NULL);
}

static int Vector_setY(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setY) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the y attribute");
        return -1;
    }

    return self -> setY(self -> parent, value, NULL);
}

static PyObject *Vector_getZ(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getZ) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the z attribute");
        return NULL;
    }

    return self -> getZ(self -> parent, NULL);
}

static int Vector_setZ(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setZ) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the z attribute");
        return -1;
    }

    return self -> setZ(self -> parent, value, NULL);
}

static PyObject *Vector_getW(Vector *self, void *Py_UNUSED(closure)) {
    if (!self -> getW) {
        PyErr_SetString(PyExc_AttributeError, "Cannot get the w attribute");
        return NULL;
    }

    return self -> getW(self -> parent, NULL);
}

static int Vector_setW(Vector *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (!self -> setW) {
        PyErr_SetString(PyExc_AttributeError, "Cannot set the w attribute");
        return -1;
    }

    return self -> setW(self -> parent, value, NULL);
}

static PyGetSetDef VectorGetSetters[] = {
    {"x", (getter) Vector_getX, (setter) Vector_setX, "The x value of the vector", NULL},
    {"y", (getter) Vector_getY, (setter) Vector_setY, "The y value of the vector", NULL},
    {"z", (getter) Vector_getZ, (setter) Vector_setZ, "The z value of the vector", NULL},
    {"w", (getter) Vector_getW, (setter) Vector_setW, "The w value of the vector", NULL},
    {"red", (getter) Vector_getRed, (setter) Vector_setRed, "The red color of the vector", NULL},
    {"green", (getter) Vector_getGreen, (setter) Vector_setGreen, "The green color of the vector", NULL},
    {"blue", (getter) Vector_getBlue, (setter) Vector_setBlue, "The blue color of the vector", NULL},
    {"alpha", (getter) Vector_getAlpha, (setter) Vector_setAlpha, "The opacity of the vector", NULL},
    {NULL}
};

static void Vector_dealloc(Vector *self) {
    Py_DECREF(self -> parent);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyObject *Vector_str(Vector *self) {
    return self -> str(self -> parent);
}

static PyObject *Vector_repr(Vector *self) {
    return self -> repr(self -> parent);
}

static PyTypeObject VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Vector",
    .tp_doc = "Class that stores a vector or position",
    .tp_basicsize = sizeof(Vector),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Vector_dealloc,
    .tp_str = (reprfunc) Vector_str,
    .tp_repr = (reprfunc) Vector_repr,
    .tp_getset = VectorGetSetters
};

static PyObject *Cursor_getX(Cursor *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos.x - window -> size.x / 2);
}

static PyObject *Cursor_getY(Cursor *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(window -> size.y / 2 - self -> pos.y);
}

static PyObject *Cursor_strPos(Cursor *self) {
    Vec2 pos = {
        self -> pos.x - window -> size.x / 2,
        window -> size.y / 2 - self -> pos.y
    };

    return strPos(pos);
}

static PyObject *Cursor_reprPos(Cursor *self) {
    Vec2 pos = {
        self -> pos.x - window -> size.x / 2,
        window -> size.y / 2 - self -> pos.y
    };

    return reprPos(pos);
}

static PyObject *Cursor_getPos(Cursor *self, void *Py_UNUSED(closure)) {
    Vector *pos = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!pos) return NULL;

    pos -> parent = (PyObject *) self;
    pos -> str = (reprfunc) Cursor_strPos;
    pos -> repr = (reprfunc) Cursor_reprPos;
    pos -> getX = (getter) Cursor_getX;
    pos -> getY = (getter) Cursor_getY;

    Py_INCREF(self);
    return (PyObject *) pos;
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

static PyGetSetDef CursorGetSetters[] = {
    {"x", (getter) Cursor_getX, NULL, "The x position of the cursor", NULL},
    {"y", (getter) Cursor_getY, NULL, "The y position of the cursor", NULL},
    {"position", (getter) Cursor_getPos, NULL, "The position of the cursor", NULL},
    {"move", (getter) Cursor_getMove, NULL, "Becomes true when the cursor moves", NULL},
    {"enter", (getter) Cursor_getEnter, NULL, "Becomes true when the cursor enters the window", NULL},
    {"leave", (getter) Cursor_getLeave, NULL, "Becomes true when the cursor leaves the window", NULL},
    {"press", (getter) Cursor_getPress, NULL, "Becomes true when a mouse button is pressed", NULL},
    {"release", (getter) Cursor_getRelease, NULL, "Becomes true when a mouse button is released", NULL},
    {NULL}
};

static PyObject *Cursor_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Cursor *self = cursor = (Cursor *) type -> tp_alloc(type, 0);

    Py_XINCREF(self);
    return (PyObject *) self;
}

static PyTypeObject CursorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Cursor",
    .tp_doc = "Class for managing the cursor or mouse",
    .tp_basicsize = sizeof(Cursor),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Cursor_new,
    .tp_getset = CursorGetSetters
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

static PyGetSetDef KeyGetSetters[] = {
    {"press", (getter) Key_getPress, NULL, "Becomes true when a key is pressed", NULL},
    {"release", (getter) Key_getRelease, NULL, "Becomes true when a key is released", NULL},
    {"repeat", (getter) Key_getRepeat, NULL, "Becomes true when the user types a character", NULL},
    {NULL}
};

static PyObject *Key_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Key *self = key = (Key *) type -> tp_alloc(type, 0);

    Set data[] = {
        [GLFW_KEY_SPACE] = {.key = "space"},
        [GLFW_KEY_APOSTROPHE] = {.key = "apostrophe"},
        [GLFW_KEY_COMMA] = {.key = "comma"},
        [GLFW_KEY_MINUS] = {.key = "minus"},
        [GLFW_KEY_PERIOD] = {.key = "period"},
        [GLFW_KEY_SLASH] = {.key = "slash"},
        [GLFW_KEY_0] = {.key = "_0"},
        [GLFW_KEY_1] = {.key = "_1"},
        [GLFW_KEY_2] = {.key = "_2"},
        [GLFW_KEY_3] = {.key = "_3"},
        [GLFW_KEY_4] = {.key = "_4"},
        [GLFW_KEY_5] = {.key = "_5"},
        [GLFW_KEY_6] = {.key = "_6"},
        [GLFW_KEY_7] = {.key = "_7"},
        [GLFW_KEY_8] = {.key = "_8"},
        [GLFW_KEY_9] = {.key = "_9"},
        [GLFW_KEY_SEMICOLON] = {.key = "semicolon"},
        [GLFW_KEY_EQUAL] = {.key = "equal"},
        [GLFW_KEY_A] = {.key = "a"},
        [GLFW_KEY_B] = {.key = "b"},
        [GLFW_KEY_C] = {.key = "c"},
        [GLFW_KEY_D] = {.key = "d"},
        [GLFW_KEY_E] = {.key = "e"},
        [GLFW_KEY_F] = {.key = "f"},
        [GLFW_KEY_G] = {.key = "g"},
        [GLFW_KEY_H] = {.key = "h"},
        [GLFW_KEY_I] = {.key = "i"},
        [GLFW_KEY_J] = {.key = "j"},
        [GLFW_KEY_K] = {.key = "k"},
        [GLFW_KEY_L] = {.key = "l"},
        [GLFW_KEY_M] = {.key = "m"},
        [GLFW_KEY_N] = {.key = "n"},
        [GLFW_KEY_O] = {.key = "o"},
        [GLFW_KEY_P] = {.key = "p"},
        [GLFW_KEY_Q] = {.key = "q"},
        [GLFW_KEY_R] = {.key = "r"},
        [GLFW_KEY_S] = {.key = "s"},
        [GLFW_KEY_T] = {.key = "t"},
        [GLFW_KEY_U] = {.key = "u"},
        [GLFW_KEY_V] = {.key = "v"},
        [GLFW_KEY_W] = {.key = "w"},
        [GLFW_KEY_X] = {.key = "x"},
        [GLFW_KEY_Y] = {.key = "y"},
        [GLFW_KEY_Z] = {.key = "z"},
        [GLFW_KEY_LEFT_BRACKET] = {.key = "left_bracket"},
        [GLFW_KEY_BACKSLASH] = {.key = "backslash"},
        [GLFW_KEY_RIGHT_BRACKET] = {.key = "right_bracket"},
        [GLFW_KEY_GRAVE_ACCENT] = {.key = "grave_accent"},
        [GLFW_KEY_ESCAPE] = {.key = "escape"},
        [GLFW_KEY_ENTER] = {.key = "enter"},
        [GLFW_KEY_TAB] = {.key = "tab"},
        [GLFW_KEY_BACKSPACE] = {.key = "backspace"},
        [GLFW_KEY_INSERT] = {.key = "insert"},
        [GLFW_KEY_DELETE] = {.key = "delete"},
        [GLFW_KEY_RIGHT] = {.key = "right"},
        [GLFW_KEY_LEFT] = {.key = "left"},
        [GLFW_KEY_DOWN] = {.key = "down"},
        [GLFW_KEY_UP] = {.key = "up"},
        [GLFW_KEY_PAGE_UP] = {.key = "page_up"},
        [GLFW_KEY_PAGE_DOWN] = {.key = "page_down"},
        [GLFW_KEY_HOME] = {.key = "home"},
        [GLFW_KEY_END] = {.key = "end"},
        [GLFW_KEY_CAPS_LOCK] = {.key = "caps_lock"},
        [GLFW_KEY_SCROLL_LOCK] = {.key = "scroll_lock"},
        [GLFW_KEY_NUM_LOCK] = {.key = "num_lock"},
        [GLFW_KEY_PRINT_SCREEN] = {.key = "print_screen"},
        [GLFW_KEY_PAUSE] = {.key = "pause"},
        [GLFW_KEY_F1] = {.key = "f1"},
        [GLFW_KEY_F2] = {.key = "f2"},
        [GLFW_KEY_F3] = {.key = "f3"},
        [GLFW_KEY_F4] = {.key = "f4"},
        [GLFW_KEY_F5] = {.key = "f5"},
        [GLFW_KEY_F6] = {.key = "f6"},
        [GLFW_KEY_F7] = {.key = "f7"},
        [GLFW_KEY_F8] = {.key = "f8"},
        [GLFW_KEY_F9] = {.key = "f9"},
        [GLFW_KEY_F10] = {.key = "f10"},
        [GLFW_KEY_F11] = {.key = "f11"},
        [GLFW_KEY_F12] = {.key = "f12"},
        [GLFW_KEY_F13] = {.key = "f13"},
        [GLFW_KEY_F14] = {.key = "f14"},
        [GLFW_KEY_F15] = {.key = "f15"},
        [GLFW_KEY_F16] = {.key = "f16"},
        [GLFW_KEY_F17] = {.key = "f17"},
        [GLFW_KEY_F18] = {.key = "f18"},
        [GLFW_KEY_F19] = {.key = "f19"},
        [GLFW_KEY_F20] = {.key = "f20"},
        [GLFW_KEY_F21] = {.key = "f21"},
        [GLFW_KEY_F22] = {.key = "f22"},
        [GLFW_KEY_F23] = {.key = "f23"},
        [GLFW_KEY_F24] = {.key = "f24"},
        [GLFW_KEY_F25] = {.key = "f25"},
        [GLFW_KEY_KP_0] = {.key = "pad_0"},
        [GLFW_KEY_KP_1] = {.key = "pad_1"},
        [GLFW_KEY_KP_2] = {.key = "pad_2"},
        [GLFW_KEY_KP_3] = {.key = "pad_3"},
        [GLFW_KEY_KP_4] = {.key = "pad_4"},
        [GLFW_KEY_KP_5] = {.key = "pad_5"},
        [GLFW_KEY_KP_6] = {.key = "pad_6"},
        [GLFW_KEY_KP_7] = {.key = "pad_7"},
        [GLFW_KEY_KP_8] = {.key = "pad_8"},
        [GLFW_KEY_KP_9] = {.key = "pad_9"},
        [GLFW_KEY_KP_DECIMAL] = {.key = "pad_decimal"},
        [GLFW_KEY_KP_DIVIDE] = {.key = "pad_divide"},
        [GLFW_KEY_KP_MULTIPLY] = {.key = "pad_multiply"},
        [GLFW_KEY_KP_SUBTRACT] = {.key = "pad_subtract"},
        [GLFW_KEY_KP_ADD] = {.key = "pad_add"},
        [GLFW_KEY_KP_ENTER] = {.key = "pad_enter"},
        [GLFW_KEY_KP_EQUAL] = {.key = "pad_equal"},
        [GLFW_KEY_LEFT_SHIFT] = {.key = "left_shift"},
        [GLFW_KEY_LEFT_CONTROL] = {.key = "left_ctrl"},
        [GLFW_KEY_LEFT_ALT] = {.key = "left_alt"},
        [GLFW_KEY_LEFT_SUPER] = {.key = "left_super"},
        [GLFW_KEY_RIGHT_SHIFT] = {.key = "right_shift"},
        [GLFW_KEY_RIGHT_CONTROL] = {.key = "right_ctrl"},
        [GLFW_KEY_RIGHT_ALT] = {.key = "right_alt"},
        [GLFW_KEY_RIGHT_SUPER] = {.key = "right_super"},
        [GLFW_KEY_MENU] = {.key = "menu"}
    };

    Py_XINCREF(self);
    memcpy(self -> keys, data, sizeof data);

    return (PyObject *) self;
}

static PyObject *Key_getattro(Key *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    
    for (int i = 0; i < GLFW_KEY_LAST + 1; i ++) {
        Set set = self -> keys[i];

        if (set.key && !strcmp(set.key, name)) {
            if (set.hold || set.release) {
                PyObject *object = PyDict_New();
                if (!object) return NULL;

                if (PyDict_SetItemString(object, "press", PyBool_FromLong(set.press)) < 0) {
                    Py_DECREF(object);
                    return NULL;
                }

                if (PyDict_SetItemString(object, "release", PyBool_FromLong(set.release)) < 0) {
                    Py_DECREF(object);
                    return NULL;
                }

                if (PyDict_SetItemString(object, "repeat", PyBool_FromLong(set.repeat)) < 0) {
                    Py_DECREF(object);
                    return NULL;
                }

                return object;
            }

            Py_RETURN_FALSE;
        }
    }

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static PyTypeObject KeyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Key",
    .tp_doc = "Class for managing keyboard input",
    .tp_basicsize = sizeof(Key),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Key_new,
    .tp_getattro = (getattrofunc) Key_getattro,
    .tp_getset = KeyGetSetters
};

static PyObject *Camera_getX(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos.x);
}

static int Camera_setX(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> pos.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Camera_getY(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos.y);
}

static int Camera_setY(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> pos.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Camera_strPos(Camera *self) {
    return strPos(self -> pos);
}

static PyObject *Camera_reprPos(Camera *self) {
    return reprPos(self -> pos);
}

static PyObject *Camera_getPos(Camera *self, void *Py_UNUSED(closure)) {
    Vector *pos = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!pos) return NULL;

    pos -> parent = (PyObject *) self;
    pos -> str = (reprfunc) Camera_strPos;
    pos -> repr = (reprfunc) Camera_reprPos;
    pos -> getX = (getter) Camera_getX;
    pos -> setX = (setter) Camera_setX;
    pos -> getY = (getter) Camera_getY;
    pos -> setY = (setter) Camera_setY;

    Py_INCREF(self);
    return (PyObject *) pos;
}

static int Camera_setPos(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setPos(value, &self -> pos) < 0)
        return -1;

    return 0;
}

static PyGetSetDef CameraGetSetters[] = {
    {"x", (getter) Camera_getX, (setter) Camera_setX, "The x position of the camera", NULL},
    {"y", (getter) Camera_getY, (setter) Camera_setY, "The y position of the camera", NULL},
    {"position", (getter) Camera_getPos, (setter) Camera_setPos, "The position of the camera", NULL},
    {NULL}
};

static PyObject *Camera_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Camera *self = camera = (Camera *) type -> tp_alloc(type, 0);
    self -> view.y = 1;

    Py_XINCREF(self);
    return (PyObject *) self;
}

static PyTypeObject CameraType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Camera",
    .tp_doc = "Class for managing the screen view",
    .tp_basicsize = sizeof(Camera),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Camera_new,
    .tp_getset = CameraGetSetters
};

static PyObject *Window_getCaption(Window *self, void *Py_UNUSED(closure)) {
    return PyUnicode_FromString(self -> caption);
}

static int Window_setCaption(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if (!(self -> caption = PyUnicode_AsUTF8(value)) && PyErr_Occurred())
        return -1;

    glfwSetWindowTitle(self -> window, self -> caption);
    return 0;
}

static PyObject *Window_getRed(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.x);
}

static int Window_setRed(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    glClearColor(
        (GLfloat) self -> color.x, (GLfloat) self -> color.y,
        (GLfloat) self -> color.z, 1);

    return 0;
}

static PyObject *Window_getGreen(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.y);
}

static int Window_setGreen(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    glClearColor(
        (GLfloat) self -> color.x, (GLfloat) self -> color.y,
        (GLfloat) self -> color.z, 1);

    return 0;
}

static PyObject *Window_getBlue(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.z);
}

static int Window_setBlue(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.z = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    glClearColor(
        (GLfloat) self -> color.x, (GLfloat) self -> color.y,
        (GLfloat) self -> color.z, 1);

    return 0;
}

static PyObject *Window_getAlpha(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.w);
}

static int Window_setAlpha(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.w = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    glfwSetWindowOpacity(self -> window, (float) constrainValue(self -> color.w, 0, 1));
    return 0;
}

static PyObject *Window_strColor(Window *self) {
    return strColor(self -> color);
}

static PyObject *Window_reprColor(Window *self) {
    return reprColor(self -> color);
}

static PyObject *Window_getColor(Window *self, void *Py_UNUSED(closure)) {
    Vector *color = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!color) return NULL;

    color -> parent = (PyObject *) self;
    color -> str = (reprfunc) Window_strColor;
    color -> repr = (reprfunc) Window_reprColor;
    color -> getX = (getter) Window_getRed;
    color -> setX = (setter) Window_setRed;
    color -> getY = (getter) Window_getGreen;
    color -> setY = (setter) Window_setGreen;
    color -> getZ = (getter) Window_getBlue;
    color -> setZ = (setter) Window_setBlue;
    color -> getW = (getter) Window_getAlpha;
    color -> setW = (setter) Window_setAlpha;

    Py_INCREF(self);
    return (PyObject *) color;
}

static int Window_setColor(Window *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setColor(value, &self -> color) < 0)
        return -1;

    glClearColor(
        (GLfloat) self -> color.x, (GLfloat) self -> color.y,
        (GLfloat) self -> color.z, 1);

    glfwSetWindowOpacity(self -> window, (float) constrainValue(self -> color.w, 0, 1));
    return 0;
}

static PyObject *Window_getWidth(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.x);
}

static PyObject *Window_getHeight(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.y);
}

static PyObject *Window_strSize(Window *self) {
    return strPos(self -> size);
}

static PyObject *Window_reprSize(Window *self) {
    return reprPos(self -> size);
}

static PyObject *Window_getSize(Window *self, void *Py_UNUSED(closure)) {
    Vector *size = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!size) return NULL;

    size -> parent = (PyObject *) self;
    size -> str = (reprfunc) Window_strSize;
    size -> repr = (reprfunc) Window_reprSize;
    size -> getX = (getter) Window_getWidth;
    size -> getY = (getter) Window_getHeight;

    Py_INCREF(self);
    return (PyObject *) size;
}

static PyObject *Window_getTop(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.y / 2);
}

static PyObject *Window_getBottom(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.y / -2);
}

static PyObject *Window_getLeft(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.x / -2);
}

static PyObject *Window_getRight(Window *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.x / 2);
}

static PyObject *Window_getResize(Window *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> resize);
}

static PyGetSetDef WindowGetSetters[] = {
    {"caption", (getter) Window_getCaption, (setter) Window_setCaption, "The name of the window", NULL},
    {"red", (getter) Window_getRed, (setter) Window_setRed, "The red color of the window", NULL},
    {"green", (getter) Window_getGreen, (setter) Window_setGreen, "The green color of the window", NULL},
    {"blue", (getter) Window_getBlue, (setter) Window_setBlue, "The blue color of the window", NULL},
    {"alpha", (getter) Window_getAlpha, (setter) Window_setAlpha, "The opacity of the window", NULL},
    {"color", (getter) Window_getColor, (setter) Window_setColor, "The color of the window", NULL},
    {"width", (getter) Window_getWidth, NULL, "The width of the window", NULL},
    {"height", (getter) Window_getHeight, NULL, "The height of the window", NULL},
    {"size", (getter) Window_getSize, NULL, "The dimensions of the window", NULL},
    {"top", (getter) Window_getTop, NULL, "The top of the window", NULL},
    {"bottom", (getter) Window_getBottom, NULL, "The bottom of the window", NULL},
    {"left", (getter) Window_getLeft, NULL, "The left of the window", NULL},
    {"right", (getter) Window_getRight, NULL, "The right of the window", NULL},
    {"resize", (getter) Window_getResize, NULL, "Becomes true when the window is being resized", NULL},
    {NULL}
};

static PyObject *Window_close(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwSetWindowShouldClose(self -> window, GLFW_TRUE);
    Py_RETURN_NONE;
}

static PyObject *Window_maximize(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwMaximizeWindow(self -> window);
    Py_RETURN_NONE;
}

static PyObject *Window_minimize(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwIconifyWindow(self -> window);
    Py_RETURN_NONE;
}

static PyObject *Window_restore(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwRestoreWindow(self -> window);
    Py_RETURN_NONE;
}

static PyObject *Window_focus(Window *self, PyObject *Py_UNUSED(ignored)) {
    glfwFocusWindow(self -> window);
    Py_RETURN_NONE;
}

static PyMethodDef WindowMethods[] = {
    {"close", (PyCFunction) Window_close, METH_NOARGS, "Close the window"},
    {"maximize", (PyCFunction) Window_maximize, METH_NOARGS, "Maximize the window"},
    {"minimize", (PyCFunction) Window_minimize, METH_NOARGS, "Minimize the window"},
    {"restore", (PyCFunction) Window_restore, METH_NOARGS, "Restore the window from maximized or minimized"},
    {"focus", (PyCFunction) Window_focus, METH_NOARGS, "Bring the window to the front"},
    {NULL}
};

static PyObject *Window_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Window *self = window = (Window *) type -> tp_alloc(type, 0);

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    self -> window = glfwCreateWindow(1, 1, "", NULL, NULL);

    if (!self -> window) {
        const char *buffer;
        glfwGetError(&buffer);
        PyErr_SetString(error, buffer);

        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(self -> window);
    glfwSetWindowSizeCallback(self -> window, windowSizeCallback);
    glfwSetFramebufferSizeCallback(self -> window, framebufferSizeCallback);
    glfwSetCursorPosCallback(self -> window, cursorPosCallback);
    glfwSetCursorEnterCallback(self -> window, cursorEnterCallback);
    glfwSetMouseButtonCallback(self -> window, mouseButtonCallback);
    glfwSetKeyCallback(self -> window, keyCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        glfwTerminate();
        PyErr_SetString(error, "Failed to initialize OpenGL");
        return NULL;
    }

    Py_XINCREF(self);
    return (PyObject *) self;
}

static int Window_init(Window *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"caption", "width", "height", "color", NULL};
    PyObject *color = NULL;

    self -> caption = "JoBase";
    self -> color.x = 1;
    self -> color.y = 1;
    self -> color.z = 1;
    self -> color.w = 1;
    self -> size.x = 640;
    self -> size.y = 480;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sddO", kwlist,
        &self -> caption, &self -> size.x,
        &self -> size.y, &color)) return -1;

    if (color && setColor(color, &self -> color) < 0)
        return -1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(
        (GLfloat) self -> color.x, (GLfloat) self -> color.y,
        (GLfloat) self -> color.z, 1);

    glfwSetWindowTitle(self -> window, self -> caption);
    glfwSetWindowSize(self -> window, (int) self -> size.x, (int) self -> size.y);
    glfwSetWindowOpacity(self -> window, (float) constrainValue(self -> color.w, 0, 1));

    return 0;
}

static PyTypeObject WindowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Window",
    .tp_doc = "Class for managing the main window",
    .tp_basicsize = sizeof(Window),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Window_new,
    .tp_init = (initproc) Window_init,
    .tp_methods = WindowMethods,
    .tp_getset = WindowGetSetters
};

static PyObject *Shape_getX(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos.x);
}

static int Shape_setX(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> pos.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getY(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos.y);
}

static int Shape_setY(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> pos.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_strPos(Shape *self) {
    return strPos(self -> pos);
}

static PyObject *Shape_reprPos(Shape *self) {
    return reprPos(self -> pos);
}

static PyObject *Shape_getPos(Shape *self, void *Py_UNUSED(closure)) {
    Vector *pos = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!pos) return NULL;

    pos -> parent = (PyObject *) self;
    pos -> str = (reprfunc) Shape_strPos;
    pos -> repr = (reprfunc) Shape_reprPos;
    pos -> getX = (getter) Shape_getX;
    pos -> setX = (setter) Shape_setX;
    pos -> getY = (getter) Shape_getY;
    pos -> setY = (setter) Shape_setY;

    Py_INCREF(self);
    return (PyObject *) pos;
}

static int Shape_setPos(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setPos(value, &self -> pos) < 0)
        return -1;

    return 0;
}

static PyObject *Shape_getScaleX(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> scale.x);
}

static int Shape_setScaleX(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> scale.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getScaleY(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> scale.y);
}

static int Shape_setScaleY(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> scale.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_strScale(Shape *self) {
    return strPos(self -> scale);
}

static PyObject *Shape_reprScale(Shape *self) {
    return reprPos(self -> scale);
}

static PyObject *Shape_getScale(Shape *self, void *Py_UNUSED(closure)) {
    Vector *scale = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!scale) return NULL;

    scale -> parent = (PyObject *) self;
    scale -> str = (reprfunc) Shape_strScale;
    scale -> repr = (reprfunc) Shape_reprScale;
    scale -> getX = (getter) Shape_getScaleX;
    scale -> setX = (setter) Shape_setScaleX;
    scale -> getY = (getter) Shape_getScaleY;
    scale -> setY = (setter) Shape_setScaleY;

    Py_INCREF(self);
    return (PyObject *) scale;
}

static int Shape_setScale(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setPos(value, &self -> scale) < 0)
        return -1;

    return 0;
}

static PyObject *Shape_getAnchorX(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> anchor.x);
}

static int Shape_setAnchorX(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> anchor.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getAnchorY(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> anchor.y);
}

static int Shape_setAnchorY(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> anchor.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_strAnchor(Shape *self) {
    return strPos(self -> anchor);
}

static PyObject *Shape_reprAnchor(Shape *self) {
    return reprPos(self -> anchor);
}

static PyObject *Shape_getAnchor(Shape *self, void *Py_UNUSED(closure)) {
    Vector *anchor = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!anchor) return NULL;

    anchor -> parent = (PyObject *) self;
    anchor -> str = (reprfunc) Shape_strAnchor;
    anchor -> repr = (reprfunc) Shape_reprAnchor;
    anchor -> getX = (getter) Shape_getAnchorX;
    anchor -> setX = (setter) Shape_setAnchorX;
    anchor -> getY = (getter) Shape_getAnchorY;
    anchor -> setY = (setter) Shape_setAnchorY;

    Py_INCREF(self);
    return (PyObject *) anchor;
}

static int Shape_setAnchor(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setPos(value, &self -> anchor) < 0)
        return -1;

    return 0;
}

static PyObject *Shape_getAngle(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> angle);
}

static int Shape_setAngle(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> angle = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getRed(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.x);
}

static int Shape_setRed(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getGreen(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.y);
}

static int Shape_setGreen(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getBlue(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.z);
}

static int Shape_setBlue(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.z = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_getAlpha(Shape *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color.w);
}

static int Shape_setAlpha(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> color.w = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Shape_strColor(Shape *self) {
    return strColor(self -> color);
}

static PyObject *Shape_reprColor(Shape *self) {
    return reprColor(self -> color);
}

static PyObject *Shape_getColor(Shape *self, void *Py_UNUSED(closure)) {
    Vector *color = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!color) return NULL;

    color -> parent = (PyObject *) self;
    color -> str = (reprfunc) Shape_strColor;
    color -> repr = (reprfunc) Shape_reprColor;
    color -> getX = (getter) Shape_getRed;
    color -> setX = (setter) Shape_setRed;
    color -> getY = (getter) Shape_getGreen;
    color -> setY = (setter) Shape_setGreen;
    color -> getZ = (getter) Shape_getBlue;
    color -> setZ = (setter) Shape_setBlue;
    color -> getW = (getter) Shape_getAlpha;
    color -> setW = (setter) Shape_setAlpha;

    Py_INCREF(self);
    return (PyObject *) color;
}

static int Shape_setColor(Shape *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setColor(value, &self -> color) < 0)
        return -1;

    return 0;
}

static PyGetSetDef ShapeGetSetters[] = {
    {"x", (getter) Shape_getX, (setter) Shape_setX, "The x position of the shape", NULL},
    {"y", (getter) Shape_getY, (setter) Shape_setY, "The y position of the shape", NULL},
    {"position", (getter) Shape_getPos, (setter) Shape_setPos, "The position of the shape", NULL},
    {"scale", (getter) Shape_getScale, (setter) Shape_setScale, "The scale of the shape", NULL},
    {"anchor", (getter) Shape_getAnchor, (setter) Shape_setAnchor, "The rotation origin of the shape", NULL},
    {"angle", (getter) Shape_getAngle, (setter) Shape_setAngle, "The angle of the shape", NULL},
    {"red", (getter) Shape_getRed, (setter) Shape_setRed, "The red color of the shape", NULL},
    {"green", (getter) Shape_getGreen, (setter) Shape_setGreen, "The green color of the shape", NULL},
    {"blue", (getter) Shape_getBlue, (setter) Shape_setBlue, "The blue color of the shape", NULL},
    {"alpha", (getter) Shape_getAlpha, (setter) Shape_setAlpha, "The opacity of the shape", NULL},
    {"color", (getter) Shape_getColor, (setter) Shape_setColor, "The color of the shape", NULL},
    {NULL}
};

static PyObject *Shape_lookAt(Shape *self, PyObject *args) {
    Shape *other;

    if (!PyArg_ParseTuple(args, "O!", &ShapeType, &other))
        return NULL;

    double angle = atan2(other -> pos.y - self -> pos.y, other -> pos.x - self -> pos.x);
    self -> angle = angle * 180 / M_PI;
    
    Py_RETURN_NONE;
}

static PyObject *Shape_moveToward(Shape *self, PyObject *args) {
    Shape *other;
    double speed = 1;

    if (!PyArg_ParseTuple(args, "O!|d", &ShapeType, &other, &speed))
        return NULL;

    double x = other -> pos.x - self -> pos.x;
    double y = other -> pos.y - self -> pos.y;

    if (hypot(x, y) < speed) {
        self -> pos.x += x;
        self -> pos.y += y;
    }

    else {
        double angle = atan2(y, x);
        self -> pos.x += cos(angle) * speed;
        self -> pos.y += sin(angle) * speed;
    }
    
    Py_RETURN_NONE;
}

static PyMethodDef ShapeMethods[] = {
    {"look_at", (PyCFunction) Shape_lookAt, METH_VARARGS, "Rotate the shape so that it looks at another object"},
    {"move_toward", (PyCFunction) Shape_moveToward, METH_VARARGS, "Move the shape toward another object"},
    {NULL}
};

static int Shape_init(Shape *self, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    self -> pos.x = 0;
    self -> pos.y = 0;

    self -> anchor.x = 0;
    self -> anchor.y = 0;

    self -> scale.x = 1;
    self -> scale.y = 1;

    self -> color.x = 0;
    self -> color.y = 0;
    self -> color.z = 0;
    self -> color.w = 1;

    return 0;
}

static PyTypeObject ShapeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Shape",
    .tp_doc = "Base class for drawing shapes",
    .tp_basicsize = sizeof(Shape),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Shape_init,
    .tp_methods = ShapeMethods,
    .tp_getset = ShapeGetSetters
};

static PyObject *Rectangle_getWidth(Rectangle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.x);
}

static int Rectangle_setWidth(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> size.x = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Rectangle_getHeight(Rectangle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size.y);
}

static int Rectangle_setHeight(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);

    if ((self -> size.y = PyFloat_AsDouble(value)) < 0 && PyErr_Occurred())
        return -1;

    return 0;
}

static PyObject *Rectangle_strSize(Rectangle *self) {
    return strPos(self -> size);
}

static PyObject *Rectangle_reprSize(Rectangle *self) {
    return reprPos(self -> size);
}

static PyObject *Rectangle_getSize(Rectangle *self, void *Py_UNUSED(closure)) {
    Vector *size = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!size) return NULL;

    size -> parent = (PyObject *) self;
    size -> str = (reprfunc) Rectangle_strSize;
    size -> repr = (reprfunc) Rectangle_reprSize;
    size -> getX = (getter) Rectangle_getWidth;
    size -> setX = (setter) Rectangle_setWidth;
    size -> getY = (getter) Rectangle_getHeight;
    size -> setY = (setter) Rectangle_setHeight;

    Py_INCREF(self);
    return (PyObject *) size;
}

static int Rectangle_setSize(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    if (setPos(value, &self -> size) < 0)
        return -1;

    return 0;
}

static PyObject *Rectangle_getLeft(Rectangle *self, void *Py_UNUSED(closure)) {
    Vec2 array[4];
    getRectanglePoly(self, array);

    return PyFloat_FromDouble(getPolyLeft(array, 4));
}

static int Rectangle_setLeft(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);
    double result = PyFloat_AsDouble(value);

    if (result < 0 && PyErr_Occurred())
        return -1;

    Vec2 array[4];
    getRectanglePoly(self, array);
    self -> shape.pos.x += result - getPolyLeft(array, 4);

    return 0;
}

static PyObject *Rectangle_getTop(Rectangle *self, void *Py_UNUSED(closure)) {
    Vec2 array[4];
    getRectanglePoly(self, array);

    return PyFloat_FromDouble(getPolyTop(array, 4));
}

static int Rectangle_setTop(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);
    double result = PyFloat_AsDouble(value);

    if (result < 0 && PyErr_Occurred())
        return -1;

    Vec2 array[4];
    getRectanglePoly(self, array);
    self -> shape.pos.y += result - getPolyTop(array, 4);

    return 0;
}

static PyObject *Rectangle_getRight(Rectangle *self, void *Py_UNUSED(closure)) {
    Vec2 array[4];
    getRectanglePoly(self, array);

    return PyFloat_FromDouble(getPolyRight(array, 4));
}

static int Rectangle_setRight(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);
    double result = PyFloat_AsDouble(value);

    if (result < 0 && PyErr_Occurred())
        return -1;

    Vec2 array[4];
    getRectanglePoly(self, array);
    self -> shape.pos.x += result - getPolyRight(array, 4);

    return 0;
}

static PyObject *Rectangle_getBottom(Rectangle *self, void *Py_UNUSED(closure)) {
    Vec2 array[4];
    getRectanglePoly(self, array);

    return PyFloat_FromDouble(getPolyBottom(array, 4));
}

static int Rectangle_setBottom(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    CHECK(value);
    double result = PyFloat_AsDouble(value);

    if (result < 0 && PyErr_Occurred())
        return -1;

    Vec2 array[4];
    getRectanglePoly(self, array);
    self -> shape.pos.y += result - getPolyBottom(array, 4);

    return 0;
}

static PyGetSetDef RectangleGetSetters[] = {
    {"width", (getter) Rectangle_getWidth, (setter) Rectangle_setWidth, "The width of the rectangle", NULL},
    {"height", (getter) Rectangle_getHeight, (setter) Rectangle_setHeight, "The height of the rectangle", NULL},
    {"size", (getter) Rectangle_getSize, (setter) Rectangle_setSize, "The dimentions of the rectangle", NULL},
    {"left", (getter) Rectangle_getLeft, (setter) Rectangle_setLeft, "The left position of the rectangle", NULL},
    {"top", (getter) Rectangle_getTop, (setter) Rectangle_setTop, "The top position of the rectangle", NULL},
    {"right", (getter) Rectangle_getRight, (setter) Rectangle_setRight, "The right position of the rectangle", NULL},
    {"bottom", (getter) Rectangle_getBottom, (setter) Rectangle_setBottom, "The bottom position of the rectangle", NULL},
    {NULL}
};

static PyObject *Rectangle_draw(Rectangle *self, PyObject *Py_UNUSED(ignored)) {
    glUniform1i(glGetUniformLocation(program, "image"), 0);

    drawRectangle(self);
    Py_RETURN_NONE;
}

static PyObject *Rectangle_collidesWith(Rectangle *self, PyObject *args) {
    PyObject *shape;
    Vec2 points[4];

    if (!PyArg_ParseTuple(args, "O", &shape))
        return NULL;

    getRectanglePoly(self, points);
    return checkShapesCollide(points, 4, shape);
}

static PyMethodDef RectangleMethods[] = {
    {"draw", (PyCFunction) Rectangle_draw, METH_NOARGS, "Draw the rectangle on the screen"},
    {"collides_with", (PyCFunction) Rectangle_collidesWith, METH_VARARGS, "Check collision with the rectangle and the other object"},
    {NULL}
};

static int Rectangle_init(Rectangle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "width", "height", "angle", "color", NULL};
    PyObject *color = NULL;

    if (ShapeType.tp_init((PyObject *) self, NULL, NULL) < 0)
        return -1;

    self -> size.x = 50;
    self -> size.y = 50;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO", kwlist, &self -> shape.pos.x,
        &self -> shape.pos.y, &self -> size.x, &self -> size.y,
        &self -> shape.angle, &color)) return -1;

    if (color && setColor(color, &self -> shape.color) < 0)
        return -1;

    return 0;
}

static PyTypeObject RectangleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Rectangle",
    .tp_doc = "Class for drawing rectangles",
    .tp_basicsize = sizeof(Rectangle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &ShapeType,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Rectangle_init,
    .tp_methods = RectangleMethods,
    .tp_getset = RectangleGetSetters
};

static PyObject *Image_draw(Image *self, PyObject *Py_UNUSED(ignored)) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self -> texture -> source);
    glUniform1i(glGetUniformLocation(program, "image"), 1);

    drawRectangle(&self -> rect);
    glBindTexture(GL_TEXTURE_2D, 0);
    Py_RETURN_NONE;
}

static PyMethodDef ImageMethods[] = {
    {"draw", (PyCFunction) Image_draw, METH_NOARGS, "Draw the image on the screen"},
    {NULL}
};

static int Image_init(Image *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", "x", "y", "angle", "width", "height", "color", NULL};
    const char *name = pathAdd("images/man.png");

    PyObject *color = NULL;
    Vec2 size = {0};

    if (ShapeType.tp_init((PyObject *) self, NULL, NULL) < 0)
        return -1;

    self -> rect.shape.color.x = 1;
    self -> rect.shape.color.y = 1;
    self -> rect.shape.color.z = 1;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sdddddO", kwlist, &name,
        &self -> rect.shape.pos.x, &self -> rect.shape.pos.y,
        &self -> rect.shape.angle, &size.x,
        &size.y, &color)) return -1;

    if (color && setColor(color, &self -> rect.shape.color) < 0)
        return -1;

    for (Texture *this = textures; this; this = this -> next)
        if (!strcmp(this -> name, name)) {
            self -> texture = this;
            self -> rect.size.x = size.x ? size.x : this -> size.x;
            self -> rect.size.y = size.y ? size.y : this -> size.y;
            return 0;
        }

    int width, height;
    stbi_uc *image = stbi_load(name, &width, &height, 0, STBI_rgb_alpha);

    if (!image) {
        PyErr_SetString(PyExc_FileNotFoundError, "Failed to load the image");
        return -1;
    }

    self -> texture = malloc(sizeof(Texture));
    glGenTextures(1, &self -> texture -> source);
    glBindTexture(GL_TEXTURE_2D, self -> texture -> source);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    self -> rect.size.x = size.x ? size.x : width;
    self -> rect.size.y = size.y ? size.y : height;
    self -> texture -> size.x = width;
    self -> texture -> size.y = height;
    self -> texture -> name = name;

    self -> texture -> next = textures;
    textures = self -> texture;

    return 0;
}

static PyTypeObject ImageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Image",
    .tp_doc = "Class for drawing images",
    .tp_basicsize = sizeof(Image),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &RectangleType,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Image_init,
    .tp_methods = ImageMethods
};

static PyObject *Module_random(PyObject *Py_UNUSED(self), PyObject *args) {
    Vec2 range;

    if (!PyArg_ParseTuple(args, "dd", &range.x, &range.y))
        return NULL;

    double section = RAND_MAX / fabs(range.y - range.x);
    return PyFloat_FromDouble(rand() / section + (range.y < range.x ? range.y : range.x));
}

static PyObject *Module_run(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(ignored)) {
    PyObject *loop = NULL;
    PyObject *module = PyDict_GetItemString(PySys_GetObject("modules"), "__main__");

    if (module && PyObject_HasAttrString(module, "loop")) {
        loop = PyObject_GetAttrString(module, "loop");
        if (!loop) return NULL;
    }

    glfwShowWindow(window -> window);

    while (!glfwWindowShouldClose(window -> window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        updateView();

        if (loop && !PyObject_CallObject(loop, NULL)) {
            memoryCleanup();
            Py_DECREF(loop);

            return NULL;
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

        for (unsigned short i = 0; i < GLFW_KEY_LAST + 1; i ++) {
            key -> keys[i].press = 0;
            key -> keys[i].release = 0;
            key -> keys[i].repeat = 0;
        }

        glfwSwapBuffers(window -> window);
        glfwPollEvents();
    }

    memoryCleanup();
    Py_XDECREF(loop);

    Py_RETURN_NONE;
}

static PyMethodDef ModuleMethods[] = {
    {"random", Module_random, METH_VARARGS, "Find a random number between two numbers"},
    {"run", Module_run, METH_NOARGS, "Activate the main game loop"},
    {NULL}
};

static int Module_exec(PyObject *self) {
    PyObject *object;

    error = PyErr_NewException("JoBase.Error", NULL, NULL);
    Py_XINCREF(error);

    if (PyModule_AddObject(self, "Error", error) < 0) {
        Py_CLEAR(error);
        Py_DECREF(self);
        return -1;
    }

    if (!glfwInit()) {
        const char *buffer;
        glfwGetError(&buffer);
        PyErr_SetString(error, buffer);

        Py_DECREF(self);
        return -1;
    }

    #define ADD(name) if ( \
        PyModule_AddObject(self, name, object) < 0) { \
            Py_XDECREF(object); \
            Py_DECREF(self); \
            return -1; \
        }

    object = PyObject_CallFunctionObjArgs((PyObject *) &CursorType, NULL);
    ADD("cursor");

    object = PyObject_CallFunctionObjArgs((PyObject *) &KeyType, NULL);
    ADD("key");

    object = PyObject_CallFunctionObjArgs((PyObject *) &CameraType, NULL);
    ADD("camera");

    object = PyObject_CallFunctionObjArgs((PyObject *) &WindowType, NULL);
    ADD("window");

    object = (PyObject *) &RectangleType;
    ADD("Rectangle");

    object = (PyObject *) &ImageType;
    ADD("Image");
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

    const GLchar *vertexSource =
        "#version 300 es\n"

        "in vec2 vertex;"
        "in vec2 coordinate;"
        "out vec2 position;"

        "uniform mat4 camera;"
        "uniform mat4 object;"

        "void main() {"
        "    gl_Position = camera * object * vec4(vertex, 0, 1);"
        "    position = coordinate;"
        "}";

    const GLchar *fragmentSource =
        "#version 300 es\n"
        "precision mediump float;"

        "in vec2 position;"
        "out vec4 fragment;"

        "uniform vec4 color;"
        "uniform sampler2D sampler;"
        "uniform int image;"

        "void main() {"
        "    fragment = image == 1 ? texture(sampler, position) * color : color;"
        "}";

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "sampler"), 0);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint buffer;
    GLfloat data[] = {-.5, .5, 0, 0, .5, .5, 1, 0, -.5, -.5, 0, 1, .5, -.5, 1, 1};

    glGenVertexArrays(1, &mesh);
    glBindVertexArray(mesh);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 64, data, GL_STATIC_DRAW);

    glVertexAttribPointer(
        glGetAttribLocation(program, "vertex"),
        2, GL_FLOAT, GL_FALSE, 16, 0);

    glVertexAttribPointer(
        glGetAttribLocation(program, "coordinate"),
        2, GL_FLOAT, GL_FALSE, 16, (void *) 8);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glDeleteBuffers(1, &buffer);

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

    object = PyUnicode_FromString(pathAdd("images/man.png"));
    ADD("MAN");

    object = PyUnicode_FromString(pathAdd("images/coin.png"));
    ADD("COIN");

    object = PyUnicode_FromString(pathAdd("images/enemy.png"));
    ADD("ENEMY");

    object = PyUnicode_FromString(pathAdd("fonts/default.ttf"));
    ADD("DEFAULT");

    object = PyUnicode_FromString(pathAdd("fonts/code.ttf"));
    ADD("CODE");

    object = PyUnicode_FromString(pathAdd("fonts/pencil.ttf"));
    ADD("PENCIL");

    object = PyUnicode_FromString(pathAdd("fonts/serif.ttf"));
    ADD("SERIF");

    object = PyUnicode_FromString(pathAdd("fonts/handwriting.ttf"));
    ADD("HANDWRITING");

    object = PyUnicode_FromString(pathAdd("fonts/typewriter.ttf"));
    ADD("TYPEWRITER");

    object = PyUnicode_FromString(pathAdd("fonts/joined.ttf"));
    ADD("JOINED");

    #undef ADD
    return 0;
}

static PyModuleDef_Slot ModuleSlots[] = {
    {Py_mod_exec, Module_exec},
    {0, NULL}
};

static struct PyModuleDef Module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "JoBase",
    .m_size = 0,
    .m_methods = ModuleMethods,
    .m_slots = ModuleSlots
};

PyMODINIT_FUNC PyInit_JoBase() {
    printf("Welcome to JoBase\n");
    srand(time(NULL));

    if (PyType_Ready(&VectorType) < 0)
        return NULL;

    if (PyType_Ready(&CursorType) < 0)
        return NULL;

    if (PyType_Ready(&KeyType) < 0)
        return NULL;

    if (PyType_Ready(&CameraType) < 0)
        return NULL;

    if (PyType_Ready(&WindowType) < 0)
        return NULL;

    if (PyType_Ready(&ShapeType) < 0)
        return NULL;

    if (PyType_Ready(&RectangleType) < 0)
        return NULL;

    if (PyType_Ready(&ImageType) < 0)
        return NULL;

    return PyModuleDef_Init(&Module);
}