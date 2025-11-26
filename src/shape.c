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

    glBufferData(GL_ARRAY_BUFFER, tessGetVertexCount(tess) * 2 * sizeof(TESSreal), tessGetVertices(tess), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, self -> indices * sizeof(TESSindex), tessGetElements(tess), GL_STATIC_DRAW);
    glBindVertexArray(0);

    return tessDeleteTess(tess), free(points), 0;
}

static Points *shape_get_points(Shape *self, void *closure) {
    return points_new(self, create);
}

static int shape_set_points(Shape *self, PyObject *value, void *closure) {
    DEL(value, "points")
    return points_set(value, self) ? -1 : create(self);
}

static Shape *shape_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Shape *self = (Shape *) BaseType.tp_new(type, args, kwds);

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

static PyObject *shape_draw(Shape *self, PyObject *args) {
    glUseProgram(shader.plain.src);
    base_matrix(&self -> base, shader.plain.obj, shader.plain.color, 1, 1);

    glBindVertexArray(self -> vao);
    glDrawElements(GL_TRIANGLES, self -> indices, GL_UNSIGNED_INT, 0);

    Py_RETURN_NONE;
}

static int shape_init(Shape *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "x", "y", "angle", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;
    BaseType.tp_init((PyObject *) self, NULL, NULL);

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

static void shape_dealloc(Shape *self) {
    GLuint buffers[] = {self -> vbo, self -> ibo};

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);

    free(self -> data);
    Py_TYPE(self) -> tp_free(self);
}

static PyGetSetDef shape_getset[] = {
    {"points", (getter) shape_get_points, (setter) shape_set_points, "The coordinates of the shape", NULL},
    {NULL}
};

static PyMethodDef shape_methods[] = {
    {"draw", (PyCFunction) shape_draw, METH_NOARGS, "Draw the shape on the screen"},
    {NULL}
};

PyTypeObject ShapeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Shape",
    .tp_doc = "Render polygons on the screen",
    .tp_basicsize = sizeof(Shape),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = (newfunc) shape_new,
    .tp_init = (initproc) shape_init,
    .tp_dealloc = (destructor) shape_dealloc,
    .tp_methods = shape_methods,
    .tp_getset = shape_getset
};