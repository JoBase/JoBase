#include "main.h"

static inline Vec2 norm(double x, double y) {
    const double len = hypot(x, y);
    Vec2 value = {len ? x / len : 0, len ? y / len : 0};

    return value;
}

static int create(Line *self) {
    size_t vtx = 0, idx = 0;

    if (self -> base.len < 2)
        return PyErr_SetString(PyExc_ValueError, "Line must contain a minimum of 2 points"), -1;

    GLfloat *verts = malloc(((self -> base.len - !self -> loop) * 10 + 4 * !self -> loop) * sizeof(GLfloat));
    GLuint *index;

    if (!verts || !(index = malloc((self -> base.len - !self -> loop) * 9 * sizeof(GLuint))))
        return PyErr_NoMemory(), -1;

    for (size_t i = 0; i < self -> base.len; i ++) {
        const Vec2 pos = self -> base.data[i];
        const size_t base = vtx / 2;

        double px = i ? self -> base.data[i - 1].x : self -> base.data[self -> base.len - 1].x;
        double py = i ? self -> base.data[i - 1].y : self -> base.data[self -> base.len - 1].y;

        double nx = i < self -> base.len - 1 ? self -> base.data[i + 1].x : self -> base.data[0].x;
        double ny = i < self -> base.len - 1 ? self -> base.data[i + 1].y : self -> base.data[0].y;

        if (!self -> loop) {
            if (!i) {
                const Vec2 vect = norm(pos.x - nx, pos.y - ny);

                px = pos.x + vect.x;
                py = pos.y + vect.y;
            }

            if (i == self -> base.len - 1) {
                const Vec2 vect = norm(pos.x - px, pos.y - py);

                nx = pos.x + vect.x;
                ny = pos.y + vect.y;
            }
        }

        const Vec2 ab = norm(pos.x - px, pos.y - py);
        const Vec2 bc = norm(nx - pos.x, ny - pos.y);
        const Vec2 tan = norm(ab.x + bc.x, ab.y + bc.y);

        const double mx = -tan.y, my = tan.x, dx = -ab.y, dy = ab.x;
        const bool inside = mx * (ab.x - bc.x) + my * (ab.y - bc.y) > 0;

        const double dot = mx * dx + my * dy, line = self -> width / 2 * (inside ? 1 : -1);
        const double rx = -(dx - 2 * dot * mx) * line, ry = -(dy - 2 * dot * my) * line;
        const bool miter = 1 / dot < self -> miter;

        const Vec2 center = {pos.x + mx * line / dot, pos.y + my * line / dot};
        const Vec2 a = {pos.x - dx * line, pos.y - dy * line};
        const Vec2 b = {pos.x + dx * line, pos.y + dy * line};

        const Vec2 first = inside ? a : miter ? center : b;
        const Vec2 last = inside ? miter ? center : b : a;

        verts[vtx ++] = first.x;
        verts[vtx ++] = first.y;
        verts[vtx ++] = last.x;
        verts[vtx ++] = last.y;

        if (i < self -> base.len - 1 || self -> loop) {
            const bool test = i == self -> base.len - 1;

            if (miter) {
                index[idx ++] = base + inside;
                index[idx ++] = base + 2;
                index[idx ++] = test ? 0 : base + 3;
                index[idx ++] = base + 1 + !inside;
                index[idx ++] = test ? 0 : base + 3;
                index[idx ++] = test ? 1 : base + 4;
            }

            else {
                index[idx ++] = base + 4;
                index[idx ++] = base + 2;
                index[idx ++] = test ? 1 : base + 6;
                index[idx ++] = base + 2 + inside * 2;
                index[idx ++] = test ? 1 : base + 5;
                index[idx ++] = test ? 0 : base + 6;
                index[idx ++] = base + inside;
                index[idx ++] = base + 2;
                index[idx ++] = base + 3;

                verts[vtx ++] = pos.x + rx;
                verts[vtx ++] = pos.y + ry;
                verts[vtx ++] = pos.x;
                verts[vtx ++] = pos.y;
            }

            verts[vtx ++] = pos.x - rx;
            verts[vtx ++] = pos.y - ry;
        }
    }

    glBindVertexArray(self -> base.vao);
    glBufferData(GL_ARRAY_BUFFER, vtx * sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx * sizeof(GLuint), index, GL_STATIC_DRAW);

    free(verts);
    free(index);

    return self -> base.indices = idx, 0;
}

static Points *line_get_points(Shape *self, void *closure) {
    return points_new(self, (int (*)(Shape *)) create);
}

static int line_set_points(Line *self, PyObject *value, void *closure) {
    DEL(value, "points")
    return points_set(value, &self -> base) ? -1 : create(self);
}

static PyObject *line_get_width(Line *self, void *closure) {
    return PyFloat_FromDouble(self -> width);
}

static int line_set_width(Line *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(self -> width = PyFloat_AsDouble(value)) ? -1 : create(self);
}

static PyObject *line_get_loop(Line *self, void *closure) {
    return PyBool_FromLong(self -> loop);
}

static int line_set_loop(Line *self, PyObject *value, void *closure) {
    DEL(value, "loop")

    const int test = PyObject_IsTrue(value);
    return test < 0 ? -1 : (self -> loop = test, create(self));
}

static PyObject *line_get_miter(Line *self, void *closure) {
    return PyFloat_FromDouble(self -> miter);
}

static int line_set_miter(Line *self, PyObject *value, void *closure) {
    DEL(value, "miter")
    return ERR(self -> miter = PyFloat_AsDouble(value)) ? -1 : create(self);
}

static PyObject *line_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    return ShapeType.tp_new(type, args, kwds);
}

static int line_init(Line *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "width", "x", "y", "angle", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;
    BaseType.tp_init((PyObject *) self, NULL, NULL);

    self -> width = 2;
    self -> loop = false;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|OddddO:Line", kwlist, &points,
        &self -> width, &self -> base.base.pos.x,
        &self -> base.base.pos.y, &self -> base.base.angle,
        &color) || points_set(points, &self -> base) || vector_set(color, (double *) &self -> base.base.color, 4))

    if (!points) {
        self -> base.len = 2;
        self -> base.data = realloc(self -> base.data, self -> base.len * sizeof(Vec2));

        if (!self -> base.data)
            return PyErr_NoMemory(), -1;

        self -> base.data[0].x = self -> base.data[0].y = -25;
        self -> base.data[1].x = self -> base.data[1].y = 25;
    }

    return create(self);
}

static PyGetSetDef line_getset[] = {
    {"points", (getter) line_get_points, (setter) line_set_points, "The coordinates of the line", NULL},
    {"width", (getter) line_get_width, (setter) line_set_width, "The thickness of the line", NULL},
    {"loop", (getter) line_get_loop, (setter) line_set_loop, "True if the line is closed in a loop", NULL},
    {"miter", (getter) line_get_miter, (setter) line_set_miter, "The miter limit of the line", NULL},
    {NULL}
};

PyTypeObject LineType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Line",
    .tp_doc = "Render lines on the screen",
    .tp_basicsize = sizeof(Line),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &ShapeType,
    .tp_new = line_new,
    .tp_init = (initproc) line_init,
    .tp_getset = line_getset
};