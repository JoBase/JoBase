#include "main.h"

static PyObject *circle_get_radius(Circle *self, void *closure) {
    return PyFloat_FromDouble(self -> diameter / 2);
}

static int circle_set_radius(Circle *self, PyObject *value, void *closure) {
    DEL(value, "radius")

    const double radius = PyFloat_AsDouble(value);
    return ERR(radius) ? -1 : (self -> diameter = radius * 2, 0);
}

static PyObject *circle_get_diameter(Circle *self, void *closure) {
    return PyFloat_FromDouble(self -> diameter);
}

static int circle_set_diameter(Circle *self, PyObject *value, void *closure) {
    DEL(value, "diameter")
    return ERR(self -> diameter = PyFloat_AsDouble(value)) ? -1 : 0;
}

static int circle_init(Circle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "diameter", "color", NULL};

    PyObject *color = NULL;

    BaseType.tp_init((PyObject *) self, NULL, NULL);
    self -> diameter = 50;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddO:Circle", kwlist,
        &self -> base.pos.x, &self -> base.pos.y,
        &self -> diameter,
        &color) ? vector_set(color, (double *) &self -> base.color, 4) : -1;
}

static PyObject *circle_draw(Circle *self, PyObject *args) {
    glUseProgram(shader.circle.src);
    base_matrix(&self -> base, shader.circle.obj, shader.circle.color, self -> diameter, self -> diameter);

    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    Py_RETURN_NONE;
}

static PyMethodDef circle_methods[] = {
    {"draw", (PyCFunction) circle_draw, METH_NOARGS, "Draw the circle on the screen"},
    {NULL}
};

static PyGetSetDef circle_getset[] = {
    {"radius", (getter) circle_get_radius, (setter) circle_set_radius, "The radius of the circle", NULL},
    {"diameter", (getter) circle_get_diameter, (setter) circle_set_diameter, "The diameter of the circle", NULL},
    {NULL}
};

PyTypeObject CircleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Circle",
    .tp_doc = "Render circles on the screen",
    .tp_basicsize = sizeof(Circle),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) circle_init,
    .tp_base = &BaseType,
    .tp_methods = circle_methods,
    .tp_getset = circle_getset
};