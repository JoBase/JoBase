#include <glad/glad.h>
#include <main.h>

static void normalize(vec2 value) {
    const double length = hypot(value[x], value[y]);

    value[x] /= length;
    value[y] /= length;
}

static void alloc(Line *self) {
    self -> base = realloc(self -> base, self -> shape.vertex / 3 * sizeof(vec2));
    self -> shape.points = realloc(self -> shape.points, self -> shape.vertex * sizeof(vec2));
}

static void parse(Line *self, size_t size) {
    size_t index = 0, start = 0;
    size_t end = self -> shape.vertex - 1;
    size_t length = self -> shape.vertex / 3;

    FOR(size_t, length) {
        vec e = self -> base[i];

        vec2 prev = {
            i ? self -> base[i - 1][x] : e[x],
            i ? self -> base[i - 1][y] : e[y]
        };

        vec2 next = {
            i < length - 1 ? self -> base[i + 1][x] : e[x],
            i < length - 1 ? self -> base[i + 1][y] : e[y]
        };

        if (e[x] == prev[x] && e[y] == prev[y]) {
            vec2 vect = {e[x] - next[x], e[y] - next[y]};
            normalize(vect);

            prev[x] += vect[x];
            prev[y] += vect[y];
        }

        if (e[x] == next[x] && e[y] == next[y]) {
            vec2 vect = {e[x] - prev[x], e[y] - prev[y]};
            normalize(vect);

            next[x] += vect[x];
            next[y] += vect[y];
        }

        vec2 ab = {e[x] - prev[x], e[y] - prev[y]};
        vec2 bc = {next[x] - e[x], next[y] - e[y]};

        normalize(ab);
        normalize(bc);

        vec2 tangent = {ab[x] + bc[x], ab[y] + bc[y]};
        vec2 point = {ab[x] - bc[x], ab[y] - bc[y]};

        normalize(tangent);
        normalize(point);

        vec2 miter = {-tangent[y], tangent[x]};
        vec2 normal = {-ab[y], ab[x]};

        const uint8_t inside = miter[x] * point[x] + miter[y] * point[y] > 0;
        size_t a, b, c, inner = start, outer = end;

        if (inside) {
            a = start, b = end, c = end - 1;
            end -= 2;
            start ++;
        }

        else {
            a = end, b = start, c = start + 1;
            start += 2;
            end --;
        }

        const double dot = miter[x] * normal[x] + miter[y] * normal[y];
        const double line = self -> width / 2 * (inside ? 1 : -1);

        self -> shape.points[a][x] = e[x] - miter[x] * line / dot;
        self -> shape.points[a][y] = e[y] - miter[y] * line / dot;
        self -> shape.points[b][x] = e[x] + normal[x] * line;
        self -> shape.points[b][y] = e[y] + normal[y] * line;
        self -> shape.points[c][x] = e[x] - (normal[x] - 2 * dot * miter[x]) * line;
        self -> shape.points[c][y] = e[y] - (normal[y] - 2 * dot * miter[y]) * line;

        if (size) {
            self -> shape.indices[index] = a;
            self -> shape.indices[index + 1] = b;
            self -> shape.indices[index + 2] = c;

            index += 3;

            if (i) {
                self -> shape.indices[index] = self -> shape.indices[index + 5] = inner - 1;
                self -> shape.indices[index + 1] = inside ? a : b;
                self -> shape.indices[index + 2] = self -> shape.indices[index + 3] = inside ? b : a;
                self -> shape.indices[index + 4] = outer + 1;
                index += 6;
            }
        }
    }

    glBindVertexArray(self -> shape.vao);
    glBufferData(GL_ARRAY_BUFFER, self -> shape.vertex * sizeof(vec2), self -> shape.points, GL_DYNAMIC_DRAW);
    if (size) glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, self -> shape.indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

static PyObject *Line_getWidth(Line *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> width);
}

static int Line_setWidth(Line *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> width = PyFloat_AsDouble(value);
    return ERR(self -> width) ? -1 : parse(self, 0), shapeBase((Shape *) self), 0;
}

static PyObject *Line_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Line *self = (Line *) shapeNew(type, args, kwds);
    return self -> base = malloc(0), (PyObject *) self;
}

static int Line_init(Line *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "width", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;

    baseInit((Base *) self);
    self -> width = 1;

    int state = PyArg_ParseTupleAndKeywords(
        args, kwds, "|OdO", kwlist, &points, &self -> width, &color);

    if (!state || (color && vectorSet(color, self -> shape.base.color, 4)))
        return -1;

    if (points) {
        if (!PySequence_Check(points)) SEQ(points)
        PyObject *seq = PySequence_Fast(points, NULL);

        self -> shape.vertex = PySequence_Fast_GET_SIZE(seq) * 3;
        alloc(self);

        FOR(size_t, self -> shape.vertex / 3) {
            PyObject *point = PySequence_Fast_GET_ITEM(seq, i);

            if (!PySequence_Check(point)) SEQ(point)
            PyObject *value = PySequence_Fast(point, NULL);

            if (PySequence_Fast_GET_SIZE(value) < 2) {
                PyErr_SetString(PyExc_ValueError, "point must contain 2 values");
                Py_DECREF(value);
                Py_DECREF(seq);
                return -1;
            }

            self -> base[i][x] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(value, x));
            self -> base[i][y] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(value, y));
            Py_DECREF(value);

            if (ERR(self -> base[i][x]) || ERR(self -> base[i][y])) {
                Py_DECREF(seq);
                return -1;
            }
        }

        Py_DECREF(seq);
    }

    else {
        self -> shape.vertex = 6;
        alloc(self);

        self -> base[0][x] = self -> base[0][y] = -25;
        self -> base[1][x] = self -> base[1][y] = 25;
    }

    size_t size = IDX(self -> shape.vertex) * sizeof(GLuint);
    self -> shape.indices = realloc(self -> shape.indices, size);
    return parse(self, size), 0;
}

static void Line_dealloc(Line *self) {
    free(self -> base);
    shapeDealloc((Shape *) self);
}

static PyGetSetDef LineGetSetters[] = {
    {"width", (getter) Line_getWidth, (setter) Line_setWidth, "thickness of the line", NULL},
    {NULL}
};

static PyMethodDef LineMethods[] = {
    {"draw", (PyCFunction) shapeDraw, METH_NOARGS, "draw the line on the screen"},
    {NULL}
};

PyTypeObject LineType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Line",
    .tp_doc = "draw lines on the screen",
    .tp_basicsize = sizeof(Line),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Line_new,
    .tp_init = (initproc) Line_init,
    .tp_dealloc = (destructor) Line_dealloc,
    .tp_getset = LineGetSetters,
    .tp_methods = LineMethods
};