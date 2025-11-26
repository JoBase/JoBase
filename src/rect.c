#include "main.h"

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
    BaseType.tp_init((PyObject *) self, NULL, NULL);

    self -> size.x = 50;
    self -> size.y = 50;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO:Rectangle", kwlist,
        &self -> base.pos.x, &self -> base.pos.y, &self -> size.x, &self -> size.y,
        &self -> base.angle, &color) ? vector_set(color, (double *) &self -> base.color, 4) : -1;
}

static PyObject *rect_draw(Rect *self, PyObject *args) {
    glUseProgram(shader.plain.src);
    base_matrix(&self -> base, shader.plain.obj, shader.plain.color, self -> size.x, self -> size.y);

    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    Py_RETURN_NONE;
}

static PyGetSetDef rect_getset[] = {
    {"width", (getter) rect_get_width, (setter) rect_set_width, "The width of the rectangle", NULL},
    {"height", (getter) rect_get_height, (setter) rect_set_height, "The height of the rectangle", NULL},
    {"size", (getter) rect_get_size, (setter) rect_set_size, "The dimentions of the rectangle", NULL},
    {NULL}
};

static PyMethodDef rect_methods[] = {
    {"draw", (PyCFunction) rect_draw, METH_NOARGS, "Draw the rectangle on the screen"},
    {NULL}
};

PyTypeObject RectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Rect",
    .tp_doc = "Render rectangles on the screen",
    .tp_basicsize = sizeof(Rect),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_base = &BaseType,
    .tp_init = (initproc) rect_init,
    .tp_getset = rect_getset,
    .tp_methods = rect_methods
};