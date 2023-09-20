#include <main.h>

static bool positive(Vec2 *points, size_t size) {
    double result = 0;

    for (size_t p = size - 1, q = 0; q < size; p = q ++)
        result += points[p].x * points[q].y - points[q].x * points[p].y;

    return result / 2 > 0;
}

static bool inside(Vec2 a, Vec2 b, Vec2 c, Vec2 p) {
    const double ab = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
    const double ca = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    const double bc = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);

    return ab >= 0 && ca >= 0 && bc >= 0;
}

static bool snip(Vec2 *poly, size_t u, size_t v, size_t w, size_t next, size_t *data) {
    Vec2 a = poly[data[u]], b = poly[data[v]], c = poly[data[w]];

    if (DBL_EPSILON > (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x))
        return false;

    for (size_t i = 0; i < next; i ++)
        if (i != u && i != v && i != w && inside(a, b, c, poly[data[i]]))
            return false;

    return true;
}

static cpVect point(Shape *self, size_t i) {
    Vec2 pos = self -> points[self -> indices[i]];
    return cpv(pos.x, pos.y);
}

static void unsafe(Shape *self) {
    cpShape *shape = self -> base.shape;

    for (size_t i = 0; i < (self -> length - 2) * 3; i += 3) {
        cpShape *next = shape;
        cpVect verts[] = {point(self, i), point(self, i + 1), point(self, i + 2)};

        if (i && !(next = cpShapeGetUserData(shape))) {
            next = cpPolyShapeNew(self -> base.body -> body, 3, verts, Base_transform(&self -> base), 0);

            cpShapeSetUserData(shape, next);
            Base_shape(&self -> base, next);
        }

        else cpPolyShapeSetVerts(next, 3, verts, Base_transform(&self -> base));
        shape = next;
    }

    Shape_reduce(self, shape);
}

static cpShape *physics(Shape *self) {
    cpShape *shape = NULL;

    for (size_t i = 0; i < (self -> length - 2) * 3; i += 3) {
        cpShape *array = shape;
        cpVect verts[] = {point(self, i), point(self, i + 1), point(self, i + 2)};

        shape = cpPolyShapeNew(self -> base.body -> body, 3, verts, Base_transform(&self -> base), 0);
        cpShapeSetUserData(shape, array);
    }

    return shape;
}

static int create(Shape *self) {
    if (self -> length < 3) {
        PyErr_SetString(PyExc_ValueError, "shape must contain a minimum of 3 corners");
        return -1;
    }

    size_t *points = malloc(self -> length * sizeof(size_t));
    size_t count = self -> length * 2, next = self -> length, index = 0, length, size;
    GLfloat *vertices = malloc(length = count * sizeof(GLfloat));

    const bool active = positive(self -> points, self -> length);
    self -> indices = realloc(self -> indices, size = (self -> length - 2) * 3 * sizeof(GLuint));

    for (size_t i = 0; i < self -> length; i ++) {
        vertices[i * 2 + x] = self -> points[i].x;
        vertices[i * 2 + y] = self -> points[i].y;
        points[i] = active ? i : (self -> length - 1) - i;
    }

    for (size_t v = next - 1; next > 2;) {
        if (count -- <= 0) {
            free(points);
            free(vertices);

            PyErr_SetString(PyExc_ValueError, "the edges of your shape overlap");
            return -1;
        }

        size_t u = next > v ? v : 0;
        v = next > u + 1 ? u + 1 : 0;
        size_t w = next > v + 1 ? v + 1 : 0;

        if (snip(self -> points, u, v, w, next, points)) {
            self -> indices[index] = points[u];
            self -> indices[index + 1] = points[v];
            self -> indices[index + 2] = points[w];

            for (size_t s = v, t = v + 1; t < next; s ++, t ++)
                points[s] = points[t];

            next --;
            count = next * 2;
            index += 3;
        }
    }

    glBindVertexArray(self -> vao);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

    glBufferData(GL_ARRAY_BUFFER, length, vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, self -> indices, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);

    free(points);
    free(vertices);

    return Base_unsafe(&self -> base), 0;
}

static Points *Shape_get_points(Shape *self, void *closure) {
    return Points_new(self, self -> reset);
}

static int Shape_set_points(Shape *self, PyObject *value, void *closure) {
    return Points_set(self, value) ? -1 : self -> reset(self);
}

static PyObject *Shape_draw(Shape *self, PyObject *args) {
    return Shape_render(self, self -> length);
}

static int Shape_init(Shape *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "x", "y", "angle", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;

    BaseType.tp_init((PyObject *) self, NULL, NULL);

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|OdddO:Shape", kwlist, &points, &self -> base.pos.x, &self -> base.pos.y,
        &self -> base.angle, &color) || Points_set(self, points) || Vector_set(color, (vec) &self -> base.color, 4))

    if (!points) {
        self -> length = 3;
        self -> points = realloc(self -> points, self -> length * sizeof(Vec2));

        self -> points[0].x = 0;
        self -> points[0].y = self -> points[1].x = 25;
        self -> points[1].y = self -> points[2].x = self -> points[2].y = -25;
    }

    return create(self);
}

PyObject *Shape_render(Shape *self, size_t size) {
    Base_matrix(&self -> base, shape, 1, 1);

    glBindVertexArray(self -> vao);
    glDrawElements(GL_TRIANGLES, (size - 2) * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    Py_RETURN_NONE;
}

Shape *Shape_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Shape *self = (Shape *) type -> tp_alloc(type, 0);

    if (self) {
        Base_buffers(&self -> vao, &self -> vbo, &self -> ibo);

        self -> base.sides = (Sides (*)(Base *)) Shape_sides;
        self -> base.physics = (cpShape *(*)(Base *)) physics;
        self -> base.unsafe = (void (*)(Base *)) unsafe;
        self -> reset = create;
    }

    return self;
}

void Shape_dealloc(Shape *self) {
    GLuint buffers[] = {self -> vbo, self -> ibo};

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);

    free(self -> points);
    free(self -> indices);

    Base_clean(&self -> base);
    ShapeType.tp_free(self);
}

Sides Shape_sides(Shape *self) {
    return Base_sides(&self -> base, self -> points, self -> length);
}

void Shape_poly(Shape *self, Vec2 *points) {
    Base_poly(&self -> base, self -> points, points, self -> length);
}

void Shape_reduce(Shape *self, cpShape *shape) {
    cpShape *next;

    while ((next = cpShapeGetUserData(shape))) {
        cpShapeSetUserData(shape, cpShapeGetUserData(next));
        cpSpaceRemoveShape(self -> base.body -> parent -> space, next);
        cpShapeFree(next);
    }
}

static PyGetSetDef Shape_getset[] = {
    {"points", (getter) Shape_get_points, (setter) Shape_set_points, "the coordinates that make up the shape", NULL},
    {NULL}
};

static PyMethodDef Shape_methods[] = {
    {"draw", (PyCFunction) Shape_draw, METH_NOARGS, "draw the shape on the screen"},
    {NULL}
};

PyTypeObject ShapeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Shape",
    .tp_doc = "draw polygons on the screen",
    .tp_basicsize = sizeof(Shape),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = (newfunc) Shape_new,
    .tp_init = (initproc) Shape_init,
    .tp_dealloc = (destructor) Shape_dealloc,
    .tp_methods = Shape_methods,
    .tp_getset = Shape_getset
};