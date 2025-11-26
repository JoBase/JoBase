#include "main.h"

static PyObject *base_get_x(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.x);
}

static int base_set_x(Base *self, PyObject *value, void *closure) {
    DEL(value, "x")
    return ERR(self -> pos.x = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *base_get_y(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.y);
}

static int base_set_y(Base *self, PyObject *value, void *closure) {
    DEL(value, "y")
    return ERR(self -> pos.y = PyFloat_AsDouble(value)) ? -1 : 0;
}

static Vector *base_get_pos(Base *self, void *closure) {
    Vector *vect = vector_new((PyObject *) self, (double *) &self -> pos, 2, NULL);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int base_set_pos(Base *self, PyObject *value, void *closure) {
    DEL(value, "pos")
    return vector_set(value, (double *) &self -> pos, 2);
}

static Vector *base_get_scale(Base *self, void *closure) {
    Vector *vect = vector_new((PyObject *) self, (double *) &self -> scale, 2, NULL);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int base_set_scale(Base *self, PyObject *value, void *closure) {
    DEL(value, "scale")
    return vector_set(value, (double *) &self -> scale, 2);
}

static Vector *base_get_anchor(Base *self, void *closure) {
    Vector *vect = vector_new((PyObject *) self, (double *) &self -> anchor, 2, NULL);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int base_set_anchor(Base *self, PyObject *value, void *closure) {
    DEL(value, "anchor")
    return vector_set(value, (double *) &self -> anchor, 2);
}

static PyObject *base_get_angle(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> angle);
}

static int base_set_angle(Base *self, PyObject *value, void *closure) {
    DEL(value, "angle")
    return ERR(self -> angle = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *base_get_red(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.x);
}

static int base_set_red(Base *self, PyObject *value, void *closure) {
    DEL(value, "red")
    return ERR(self -> color.x = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *base_get_green(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.y);
}

static int base_set_green(Base *self, PyObject *value, void *closure) {
    DEL(value, "green")
    return ERR(self -> color.y = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *base_get_blue(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.z);
}

static int base_set_blue(Base *self, PyObject *value, void *closure) {
    DEL(value, "blue")
    return ERR(self -> color.z = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *base_get_alpha(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.w);
}

static int base_set_alpha(Base *self, PyObject *value, void *closure) {
    DEL(value, "alpha")
    return ERR(self -> color.w = PyFloat_AsDouble(value)) ? -1 : 0;
}

static Vector *base_get_color(Base *self, void *closure) {
    Vector *vect = vector_new((PyObject *) self, (double *) &self -> color, 2, NULL);

    if (vect) {
        vect -> names[r] = 'r';
        vect -> names[g] = 'g';
        vect -> names[b] = 'b';
        vect -> names[a] = 'a';
    }

    return vect;
}

static int base_set_color(Base *self, PyObject *value, void *closure) {
    DEL(value, "color")
    return vector_set(value, (double *) &self -> color, 4);
}

static int base_init(Base *self, PyObject *args, PyObject *kwds) {
    self -> scale.x = 1;
    self -> scale.y = 1;

    self -> anchor.x = 0;
    self -> anchor.y = 0;

    self -> pos.x = 0;
    self -> pos.y = 0;

    self -> color.x = 0;
    self -> color.y = 0;
    self -> color.z = 0;
    self -> color.w = 1;

    return self -> angle = 0;
}

void base_matrix(Base *self, GLint obj, GLint color, double width, double height) {
    const double sine = sin(self -> angle * M_PI / 180);
    const double cosine = cos(self -> angle * M_PI / 180);

    GLfloat matrix[] = {
        width * self -> scale.x * cosine, width * self -> scale.x * sine, 0,
        height * self -> scale.y * -sine, height * self -> scale.y * cosine, 0,
        self -> anchor.x * cosine + self -> anchor.y * -sine + self -> pos.x,
        self -> anchor.x * sine + self -> anchor.y * cosine + self -> pos.y, 1
    };

    glUniformMatrix3fv(obj, 1, GL_FALSE, matrix);
    glUniform4f(color, self -> color.x, self -> color.y, self -> color.z, self -> color.w);
}

static PyGetSetDef base_getset[] = {
    {"x", (getter) base_get_x, (setter) base_set_x, "The x position of the object", NULL},
    {"y", (getter) base_get_y, (setter) base_set_y, "The y position of the object", NULL},
    {"pos", (getter) base_get_pos, (setter) base_set_pos, "The position of the object", NULL},
    {"position", (getter) base_get_pos, (setter) base_set_pos, "The position of the object", NULL},
    {"scale", (getter) base_get_scale, (setter) base_set_scale, "The scale of the object", NULL},
    {"anchor", (getter) base_get_anchor, (setter) base_set_anchor, "The rotational offset of the object", NULL},
    {"angle", (getter) base_get_angle, (setter) base_set_angle, "The angle of the object", NULL},
    {"red", (getter) base_get_red, (setter) base_set_red, "The red color of the object", NULL},
    {"green", (getter) base_get_green, (setter) base_set_green, "The green color of the object", NULL},
    {"blue", (getter) base_get_blue, (setter) base_set_blue, "The blue color of the object", NULL},
    {"alpha", (getter) base_get_alpha, (setter) base_set_alpha, "The alpha color of the object", NULL},
    {"color", (getter) base_get_color, (setter) base_set_color, "The color of the object", NULL},
    {NULL}
};

PyTypeObject BaseType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Base",
    .tp_doc = "The root class for rendering things",
    .tp_basicsize = sizeof(Base),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) base_init,
    .tp_getset = base_getset
};