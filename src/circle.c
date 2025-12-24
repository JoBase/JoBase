#include "main.h"

static void draw(Circle *self) {
    glUseProgram(shader.circle.src);
    base_matrix(&self -> base, shader.circle.obj, shader.circle.color, self -> diameter, self -> diameter);

    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

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

    base_data.type -> tp_init((PyObject *) self, NULL, NULL);
    self -> diameter = 50;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddO:Circle", kwlist,
        &self -> base.pos.x, &self -> base.pos.y,
        &self -> diameter,
        &color) ? vector_set(color, (double *) &self -> base.color, 4) : -1;
}

static PyObject *circle_draw(Circle *self, PyObject *args) {
    draw(self);
    Py_RETURN_NONE;
}

static PyObject *circle_blit(Circle *self, PyObject *item) {
    if (screen_bind(item))
        return NULL;

    draw(self);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Py_RETURN_NONE;
}

static PyObject *circle_collide(Circle *self, PyObject *item) {
    const Vec2 a = circle_pos(self);
    const double rad = base_radius((Base *) self, self -> diameter);

    if (PyObject_TypeCheck(item, rect_data.type)) {
        Vec2 b[4];
        base_rect((Base *) item, b, ((Rect *) item) -> size.x, ((Rect *) item) -> size.y);

        return PyBool_FromLong(collide_poly_circle(b, 4, self));
    }

    if (PyObject_TypeCheck(item, circle_data.type)) {
        const double width = rad + base_radius((Base *) item, ((Circle *) item) -> diameter);
        return PyBool_FromLong(collide_circle_point(a, width, circle_pos((Circle *) item)));
    }

    if (PyObject_TypeCheck(item, line_data.type)) {
        const int res = collide_line_point((Line *) item, a, rad);
        return res < 0 ? NULL : PyBool_FromLong(res);
    }

    if (PyObject_TypeCheck(item, shape_data.type)) {
        Vec2 *b = shape_points((Shape *) item);

        if (b) {
            const bool res = collide_poly_circle(b, ((Shape *) item) -> len, (Circle *) item);
            return free(b), PyBool_FromLong(res);
        }

        return NULL;
    }

    if (Py_TYPE(item) == mouse_data.type)
        return PyBool_FromLong(collide_circle_point(a, rad, mouse.pos));

    return PyErr_Format(PyExc_TypeError, "Invalid type '%s'", Py_TYPE(item) -> tp_name), NULL;
}

Vec2 circle_pos(Circle *self) {
    const double x = cos(self -> base.angle);
    const double y = sin(self -> base.angle);

    Vec2 pos = {
        self -> base.pos.x + self -> base.anchor.x * x - self -> base.anchor.y * y,
        self -> base.pos.y + self -> base.anchor.y * x + self -> base.anchor.x * y
    };

    return pos;
}

static PyMethodDef circle_methods[] = {
    {"draw", (PyCFunction) circle_draw, METH_NOARGS, "Draw the circle on the screen"},
    {"blit", (PyCFunction) circle_blit, METH_O, "Render the circle to an offscreen surface"},
    {"collide", (PyCFunction) circle_collide, METH_O, "Detect collision with another object"},
    {NULL}
};

static PyGetSetDef circle_getset[] = {
    {"radius", (getter) circle_get_radius, (setter) circle_set_radius, "The radius of the circle", NULL},
    {"diameter", (getter) circle_get_diameter, (setter) circle_set_diameter, "The diameter of the circle", NULL},
    {NULL}
};

static PyType_Slot circle_slots[] = {
    {Py_tp_doc, "Render circles on the screen"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_init, circle_init},
    {Py_tp_getset, circle_getset},
    {Py_tp_methods, circle_methods},
    {0}
};

Spec circle_data = {{"Circle", sizeof(Circle), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, circle_slots}};