#include "main.h"

static void draw(Rect *self) {
    glUseProgram(shader.plain.src);
    base_matrix(&self -> base, shader.plain.obj, shader.plain.color, self -> size.x, self -> size.y);

    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static PyObject *rect_get_top(Rect *self, void *closure) {
    return PyFloat_FromDouble(rect_y((Base *) self, self -> size.x, self -> size.y, 1));
}

static int rect_set_top(Rect *self, PyObject *value, void *closure) {
    DEL(value, "top")

    const double res = PyFloat_AsDouble(value);
    return ERR(res) ? -1 : (self -> base.pos.y += res - rect_y((Base *) self, self -> size.x, self -> size.y, 1), 0);
}

static PyObject *rect_get_right(Rect *self, void *closure) {
    return PyFloat_FromDouble(rect_x((Base *) self, self -> size.x, self -> size.y, 1));
}

static int rect_set_right(Rect *self, PyObject *value, void *closure) {
    DEL(value, "right")

    const double res = PyFloat_AsDouble(value);
    return ERR(res) ? -1 : (self -> base.pos.x += res - rect_x((Base *) self, self -> size.x, self -> size.y, 1), 0);
}

static PyObject *rect_get_bottom(Rect *self, void *closure) {
    return PyFloat_FromDouble(rect_y((Base *) self, self -> size.x, self -> size.y, -1));
}

static int rect_set_bottom(Rect *self, PyObject *value, void *closure) {
    DEL(value, "bottom")

    const double res = PyFloat_AsDouble(value);
    return ERR(res) ? -1 : (self -> base.pos.y += res - rect_y((Base *) self, self -> size.x, self -> size.y, -1), 0);
}

static PyObject *rect_get_left(Rect *self, void *closure) {
    return PyFloat_FromDouble(rect_x((Base *) self, self -> size.x, self -> size.y, -1));
}

static int rect_set_left(Rect *self, PyObject *value, void *closure) {
    DEL(value, "left")

    const double res = PyFloat_AsDouble(value);
    return ERR(res) ? -1 : (self -> base.pos.x += res - rect_x((Base *) self, self -> size.x, self -> size.y, -1), 0);
}

static PyObject *rect_get_width(Rect *self, void *closure) {
    return PyFloat_FromDouble(self -> size.x);
}

static int rect_set_width(Rect *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(self -> size.x = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *rect_get_height(Rect *self, void *closure) {
    return PyFloat_FromDouble(self -> size.y);
}

static int rect_set_height(Rect *self, PyObject *value, void *closure) {
    DEL(value, "height")
    return ERR(self -> size.y = PyFloat_AsDouble(value)) ? -1 : 0;
}

static Vector *rect_get_size(Rect *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &self -> size, 2, NULL);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int rect_set_size(Rect *self, PyObject *value, void *closure) {
    DEL(value, "size")
    return vector_set(value, (double *) &self -> size, 2);
}

static int rect_init(Rect *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "width", "height", "angle", "color", NULL};
    PyObject *color = NULL;

    base_data.type -> tp_init((PyObject *) self, NULL, NULL);
    self -> size.x = 50;
    self -> size.y = 50;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO:Rectangle", kwlist,
        &self -> base.pos.x, &self -> base.pos.y, &self -> size.x, &self -> size.y,
        &self -> base.angle, &color) ? vector_set(color, (double *) &self -> base.color, 4) : -1;
}

static PyObject *rect_draw(Rect *self, PyObject *args) {
    draw(self);
    Py_RETURN_NONE;
}

static PyObject *rect_blit(Rect *self, PyObject *item) {
    return screen_bind((Base *) self, item, (void (*)(Base *)) draw);
}

PyObject *rect_intersect(PyObject *item, Vec2 *poly) {
    if (PyObject_TypeCheck(item, rect_data.type)) {
        Vec2 b[4];
        base_rect((Base *) item, b, ((Rect *) item) -> size.x, ((Rect *) item) -> size.y);

        return PyBool_FromLong(collide_poly_poly(poly, 4, b, 4));
    }

    if (PyObject_TypeCheck(item, circle_data.type))
        return PyBool_FromLong(collide_poly_circle(poly, 4, (Circle *) item));

    if (PyObject_TypeCheck(item, line_data.type)) {
        const int res = collide_line_poly((Line *) item, poly, 4);
        return res < 0 ? NULL : PyBool_FromLong(res);
    }

    if (PyObject_TypeCheck(item, shape_data.type)) {
        Vec2 *b = shape_points((Shape *) item);

        if (b) {
            const bool res = collide_poly_poly(poly, 4, b, ((Shape *) item) -> len);
            return free(b), PyBool_FromLong(res);
        }

        return NULL;
    }

    if (Py_TYPE(item) == mouse_data.type)
        return PyBool_FromLong(collide_poly_point(poly, 4, mouse.pos));

    return PyErr_Format(PyExc_TypeError, "Invalid type '%s'", Py_TYPE(item) -> tp_name), NULL;
}

double rect_y(Base *self, double w, double h, char dir) {
    const double x = cos(self -> angle * M_PI / 180);
    const double y = sin(self -> angle * M_PI / 180);
    const double pos = self -> pos.y + self -> anchor.y * x + self -> anchor.x * y;

    return pos + (fabs(w * self -> scale.x / 2 * y) + fabs(h * self -> scale.y / 2 * x)) * dir;
}

double rect_x(Base *self, double w, double h, char dir) {
    const double x = cos(self -> angle * M_PI / 180);
    const double y = sin(self -> angle * M_PI / 180);
    const double pos = self -> pos.x + self -> anchor.x * x - self -> anchor.y * y;

    return pos + (fabs(w * self -> scale.x / 2 * x) + fabs(h * self -> scale.y / 2 * y)) * dir;
}

static PyObject *rect_collide(Rect *self, PyObject *item) {
    Vec2 poly[4];
    base_rect((Base *) self, poly, self -> size.x, self -> size.y);

    return rect_intersect(item, poly);
}

static PyGetSetDef rect_getset[] = {
    {"width", (getter) rect_get_width, (setter) rect_set_width, "The width of the rectangle", NULL},
    {"height", (getter) rect_get_height, (setter) rect_set_height, "The height of the rectangle", NULL},
    {"size", (getter) rect_get_size, (setter) rect_set_size, "The dimensions of the rectangle", NULL},
    {"top", (getter) rect_get_top, (setter) rect_set_top, "The top position of the rectangle", NULL},
    {"right", (getter) rect_get_right, (setter) rect_set_right, "The right position of the rectangle", NULL},
    {"bottom", (getter) rect_get_bottom, (setter) rect_set_bottom, "The bottom position of the rectangle", NULL},
    {"left", (getter) rect_get_left, (setter) rect_set_left, "The left position of the rectangle", NULL},
    {NULL}
};

static PyMethodDef rect_methods[] = {
    {"draw", (PyCFunction) rect_draw, METH_NOARGS, "Draw the rectangle on the screen"},
    {"blit", (PyCFunction) rect_blit, METH_O, "Draw the rectangle to an offscreen surface"},
    {"collide", (PyCFunction) rect_collide, METH_O, "Detect collision with another object"},
    {NULL}
};

static PyType_Slot rect_slots[] = {
    {Py_tp_doc, "Render rectangles on the screen"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_init, rect_init},
    {Py_tp_getset, rect_getset},
    {Py_tp_methods, rect_methods},
    {0}
};

Spec rect_data = {{"Rect", sizeof(Rect), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, rect_slots}};