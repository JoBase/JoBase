#include "main.h"

static int create(Shape *self) {
    if (self -> len < 3)
        return PyErr_SetString(PyExc_ValueError, "Shape must contain a minimum of 3 points"), -1;

    TESSreal *points = malloc(self -> len * sizeof(TESSreal) * 2);
    TESStesselator *tess;

    if (!points || !(tess = tessNewTess(NULL)))
        return PyErr_NoMemory(), -1;

    for (size_t i = 0; i < self -> len; i ++) {
        points[i * 2] = self -> data[i].x;
        points[i * 2 + 1] = self -> data[i].y;
    }

    tessAddContour(tess, 2, points, sizeof(TESSreal) * 2, self -> len);

    if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, NULL)) {
        if (tessGetStatus(tess) == TESS_STATUS_OUT_OF_MEMORY) PyErr_NoMemory();
        else PyErr_SetString(PyExc_ValueError, "Invalid shape");

        return tessDeleteTess(tess), free(points), -1;
    }

    glBindVertexArray(self -> vao);
    self -> indices = tessGetElementCount(tess) * 3;

    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

    glBufferData(GL_ARRAY_BUFFER, tessGetVertexCount(tess) * 2 * sizeof(TESSreal), tessGetVertices(tess), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, self -> indices * sizeof(TESSindex), tessGetElements(tess), GL_STATIC_DRAW);

    return tessDeleteTess(tess), free(points), 0;
}

static void draw(Shape *self) {
    glUseProgram(shader.plain.src);
    base_matrix(&self -> base, shader.plain.obj, shader.plain.color, 1, 1);

    glBindVertexArray(self -> vao);
    glDrawElements(GL_TRIANGLES, self -> indices, GL_UNSIGNED_INT, 0);
}

static Points *shape_get_points(Shape *self, void *closure) {
    return points_new(self, create);
}

static int shape_set_points(Shape *self, PyObject *value, void *closure) {
    DEL(value, "points")
    return points_set(value, self) ? -1 : create(self);
}

static Shape *shape_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Shape *self = (Shape *) base_data.type -> tp_new(type, args, kwds);

    if (self) {
        GLuint buffers[2];

        glGenVertexArrays(1, &self -> vao);
        glGenBuffers(2, buffers);

        self -> vbo = buffers[0];
        self -> ibo = buffers[1];

        glBindVertexArray(self -> vao);
        glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    }

    return self;
}

static int shape_init(Shape *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "x", "y", "angle", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;
    base_data.type -> tp_init((PyObject *) self, NULL, NULL);

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|OdddO:Shape", kwlist, &points,
        &self -> base.pos.x, &self -> base.pos.y, &self -> base.angle,
        &color) || points_set(points, self) || vector_set(color, (double *) &self -> base.color, 4))

    if (!points) {
        self -> len = 3;
        self -> data = realloc(self -> data, self -> len * sizeof(Vec2));

        if (!self -> data)
            return PyErr_NoMemory(), -1;

        self -> data[0].x = 0;
        self -> data[0].y = self -> data[1].x = 25;
        self -> data[1].y = self -> data[2].x = self -> data[2].y = -25;
    }

    return create(self);
}

static PyObject *shape_draw(Shape *self, PyObject *args) {
    draw(self);
    Py_RETURN_NONE;
}

static PyObject *shape_blit(Shape *self, PyObject *item) {
    if (screen_bind(item))
        return NULL;

    draw(self);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Py_RETURN_NONE;
}

static void shape_dealloc(Shape *self) {
    GLuint buffers[] = {self -> vbo, self -> ibo};

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);

    free(self -> data);
    Py_TYPE(self) -> tp_free(self);
}

static PyObject *shape_collide(Shape *self, PyObject *item) {
    Vec2 *a = shape_points(self);

    if (!a)
        return NULL;

    if (PyObject_TypeCheck(item, rect_data.type)) {
        Vec2 b[4];
        base_rect((Base *) item, b, ((Rect *) item) -> size.x, ((Rect *) item) -> size.y);

        const bool res = collide_poly_poly(a, self -> len, b, 4);
        return free(a), PyBool_FromLong(res);
    }

    if (PyObject_TypeCheck(item, circle_data.type)) {
        const bool res = collide_poly_circle(a, self -> len, (Circle *) item);
        return free(a), PyBool_FromLong(res);
    }

    if (PyObject_TypeCheck(item, line_data.type)) {
        const int res = collide_line_poly((Line *) item, a, self -> len);
        return free(a), res < 0 ? NULL : PyBool_FromLong(res);
    }

    if (PyObject_TypeCheck(item, shape_data.type)) {
        Vec2 *b = shape_points((Shape *) item);

        if (b) {
            const bool res = collide_poly_poly(a, self -> len, b, ((Shape *) item) -> len);
            return free(a), free(b), PyBool_FromLong(res);
        }

        return NULL;
    }

    if (Py_TYPE(item) == mouse_data.type) {
        const bool res = collide_poly_point(a, self -> len, mouse.pos);
        return free(a), PyBool_FromLong(res);
    }

    return PyErr_Format(PyExc_TypeError, "Invalid type '%s'", Py_TYPE(item) -> tp_name), NULL;
}

Vec2 *shape_points(Shape *self) {
    Vec2 *points = malloc(self -> len * sizeof(Vec2));

    if (points) {
        base_trans((Base *) self, self -> data, points, self -> len);
        return points;
    }

    return PyErr_NoMemory(), NULL;
}

static PyGetSetDef shape_getset[] = {
    {"points", (getter) shape_get_points, (setter) shape_set_points, "The coordinates of the shape", NULL},
    {NULL}
};

static PyMethodDef shape_methods[] = {
    {"draw", (PyCFunction) shape_draw, METH_NOARGS, "Draw the shape on the screen"},
    {"blit", (PyCFunction) shape_blit, METH_O, "Render the shape to an offscreen surface"},
    {"collide", (PyCFunction) shape_collide, METH_O, "Detect collision with another object"},
    {NULL}
};

static PyType_Slot shape_slots[] = {
    {Py_tp_doc, "Render polygons on the screen"},
    {Py_tp_new, shape_new},
    {Py_tp_init, shape_init},
    {Py_tp_dealloc, shape_dealloc},
    {Py_tp_getset, shape_getset},
    {Py_tp_methods, shape_methods},
    {0}
};

Spec shape_data = {{"Shape", sizeof(Shape), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, shape_slots}};