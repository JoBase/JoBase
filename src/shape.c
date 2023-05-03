#include <main.h>

static cpVect *vertices(Shape *self) {
    cpVect *verts = malloc(self -> vertex * sizeof(cpVect));

    FOR(size_t, self -> vertex) {
        verts[i].x = self -> points[i][x];
        verts[i].y = self -> points[i][y];
    }

    return verts;
}

static void new(Shape *self) {
    cpVect *verts = vertices(self);

    *self -> base.shapes = cpPolyShapeNew(self -> base.body, self -> vertex, verts, cpTransformNew(1, 0, 0, 1, 0, 0), 0);
    self -> base.length = 1;
    free(verts);
}

static cpFloat moment(Shape *self) {
    cpVect *verts = vertices(self);
    cpFloat moment = cpMomentForPoly(cpBodyGetMass(self -> base.body), self -> vertex, verts, cpv(0, 0), 0);

    return free(verts), moment;
}

static bool positive(poly poly, size_t size) {
    double result = 0;

    for (size_t p = size - 1, q = 0; q < size; p = q ++)
        result += poly[p][x] * poly[q][y] - poly[q][x] * poly[p][y];

    return result / 2 > 0;
}

static bool inside(vec2 a, vec2 b, vec2 c, vec2 p) {
    const double ab = (c[x] - b[x]) * (p[y] - b[y]) - (c[y] - b[y]) * (p[x] - b[x]);
    const double ca = (b[x] - a[x]) * (p[y] - a[y]) - (b[y] - a[y]) * (p[x] - a[x]);
    const double bc = (a[x] - c[x]) * (p[y] - c[y]) - (a[y] - c[y]) * (p[x] - c[x]);

    return ab >= 0 && ca >= 0 && bc >= 0;
}

static bool snip(poly poly, size_t u, size_t v, size_t w, size_t next, size_t *verts) {
    vec a = poly[verts[u]];
    vec b = poly[verts[v]];
    vec c = poly[verts[w]];

    if (DBL_EPSILON > (b[x] - a[x]) * (c[y] - a[y]) - (b[y] - a[y]) * (c[x] - a[x]))
        return 0;

    FOR(size_t, next) {
        if (i == u || i == v || i == w) continue;
        if (inside(a, b, c, poly[verts[i]])) return false;
    }

    return true;
}

static PyObject *Shape_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Base *self = shapeNew(type);

    self -> new = (void *)(Base *) new;
    self -> moment = (cpFloat (*)(Base *)) moment;
    self -> top = (double (*)(Base *)) shapeTop;
    self -> bottom = (double (*)(Base *)) shapeBottom;
    self -> left = (double (*)(Base *)) shapeLeft;
    self -> right = (double (*)(Base *)) shapeRight;

    return (PyObject *) self;
}

static int Shape_init(Shape *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "x", "y", "angle", "color", NULL};
    double angle = 0;

    PyObject *color = NULL;
    PyObject *points = NULL;
    baseInit((Base *) self);

    int state = PyArg_ParseTupleAndKeywords(
        args, kwds, "|OdddO", kwlist, &points, &self -> base.pos[x],
        &self -> base.pos[y], &angle, &color);

    if (!state || (color && vectorSet(color, self -> base.color, 4)) || shapeParse(self, points))
        return -1;

    if (!points) {
        self -> vertex = 3;
        self -> points = realloc(self -> points, self -> vertex * sizeof(vec2));

        self -> points[0][x] = 0;
        self -> points[0][y] = self -> points[1][x] = 25;
        self -> points[1][y] = self -> points[2][x] = self -> points[2][y] = -25;
    }

    if (self -> vertex > 2) {
        GLfloat *points = malloc(self -> vertex * 2 * sizeof(GLfloat));
        GLuint *indices = malloc(IDX(self -> vertex) * sizeof(GLuint));

        size_t *verts = malloc(self -> vertex * sizeof(size_t));
        size_t count = self -> vertex * 2, next = self -> vertex, index = 0;

        if (positive(self -> points, self -> vertex)) FOR(size_t, self -> vertex) {
            points[i * 2] = self -> points[i][x];
            points[i * 2 + 1] = self -> points[i][y];
            verts[i] = i;
        }

        else FOR(size_t, self -> vertex) {
            points[i * 2] = self -> points[i][x];
            points[i * 2 + 1] = self -> points[i][y];
            verts[i] = (self -> vertex - 1) - i;
        }

        for (size_t v = next - 1; next > 2;) {
            if (count -- <= 0) {
                PyErr_SetString(PyExc_ValueError, "failed to understand shape - probably because the edges overlap");
                return free(verts), -1;
            }

            size_t u = next > v ? v : 0;
            v = next > u + 1 ? u + 1 : 0;
            size_t w = next > v + 1 ? v + 1 : 0;

            if (snip(self -> points, u, v, w, next, verts)) {
                indices[index] = verts[u];
                indices[index + 1] = verts[v];
                indices[index + 2] = verts[w];

                for (size_t s = v, t = v + 1; t < next; s ++, t ++)
                    verts[s] = verts[t];

                next --;
                count = next * 2;
                index += 3;
            }
        }

        free(verts);
        glBindVertexArray(self -> vao);
        glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

        glBufferData(GL_ARRAY_BUFFER, self -> vertex * 2 * sizeof(GLfloat), points, GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, IDX(self -> vertex) * sizeof(GLuint), indices, GL_DYNAMIC_DRAW);
        glBindVertexArray(0);
    }

    return baseStart((Base *) self, angle), 0;
}

static PyMethodDef ShapeMethods[] = {
    {"draw", (PyCFunction) shapeDraw, METH_NOARGS, "draw the shape on the screen"},
    {NULL}
};

void shapeDealloc(Shape *self) {
    GLuint buffers[] = {self -> vbo, self -> ibo};
    free(self -> points);

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);
    baseDealloc((Base *) self);
}

PyObject *shapeDraw(Shape *self, PyObject *Py_UNUSED(ignored)) {
    baseMatrix((Base *) self, 1, 1);

    glBindVertexArray(self -> vao);
    glUniform1i(uniform[img], SHAPE);
    glDrawElements(GL_TRIANGLES, IDX(self -> vertex), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    Py_RETURN_NONE;
}

Base *shapeNew(PyTypeObject *type) {
    Shape *self = (Shape *) baseNew(type, 1);
    self -> points = malloc(0);

    buffers(&self -> vao, &self -> vbo, &self -> ibo);
    return (Base *) self;
}

poly shapePoly(Shape *self) {
    poly poly = malloc(self -> vertex * sizeof(vec2));

    FOR(size_t, self -> vertex) {
        poly[i][x] = self -> points[i][x] + self -> base.anchor[x];
        poly[i][y] = self -> points[i][y] + self -> base.anchor[y];
    }

    rotate(poly, self -> vertex, cpBodyGetAngle(self -> base.body), self -> base.pos);
    return poly;
}

int shapeParse(Shape *self, PyObject *points) {
    if (!points) return 0;
    if (!PySequence_Check(points)) SEQ(points)

    PyObject *seq = PySequence_Fast(points, NULL);
    self -> vertex = PySequence_Fast_GET_SIZE(seq);
    self -> points = realloc(self -> points, self -> vertex * sizeof(vec2));

    FOR(size_t, self -> vertex) {
        PyObject *point = PySequence_Fast_GET_ITEM(seq, i);

        if (!PySequence_Check(point)) SEQ(point)
        PyObject *value = PySequence_Fast(point, NULL);

        if (PySequence_Fast_GET_SIZE(value) < 2) {
            PyErr_SetString(PyExc_ValueError, "point must contain 2 values");
            Py_DECREF(value);
            Py_DECREF(seq);
            return -1;
        }

        self -> points[i][x] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(value, x));
        self -> points[i][y] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(value, y));
        Py_DECREF(value);

        if (ERR(self -> points[i][x]) || ERR(self -> points[i][y])) {
            Py_DECREF(seq);
            return -1;
        }
    }

    Py_DECREF(seq);
    return 0;
}

double shapeTop(Shape *self) {
    vec2 *poly = shapePoly(self);

    const double value = getTop(poly, self -> vertex);
    return free(poly), value;
}

double shapeBottom(Shape *self) {
    vec2 *poly = shapePoly(self);

    const double value = getBottom(poly, self -> vertex);
    return free(poly), value;
}

double shapeLeft(Shape *self) {
    vec2 *poly = shapePoly(self);

    const double value = getLeft(poly, self -> vertex);
    return free(poly), value;
}

double shapeRight(Shape *self) {
    vec2 *poly = shapePoly(self);

    const double value = getRight(poly, self -> vertex);
    return free(poly), value;
}

PyTypeObject ShapeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Shape",
    .tp_doc = "draw polygons on the screen",
    .tp_basicsize = sizeof(Shape),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Shape_new,
    .tp_init = (initproc) Shape_init,
    .tp_dealloc = (destructor) shapeDealloc,
    .tp_methods = ShapeMethods
};