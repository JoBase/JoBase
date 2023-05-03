#include <main.h>

static void normalize(vec2 value) {
    const double length = hypot(value[x], value[y]);

    value[x] /= length;
    value[y] /= length;
}

static void new(Line *self) {
    self -> shape.base.length = self -> shape.vertex - 1;
    self -> shape.base.shapes = realloc(self -> shape.base.shapes, self -> shape.base.length * sizeof NULL);

    FOR(size_t, self -> shape.base.length) {
        cpVect a = cpv(self -> shape.points[i][x], self -> shape.points[i][y]);
        cpVect b = cpv(self -> shape.points[i + 1][x], self -> shape.points[i + 1][y]);

        self -> shape.base.shapes[i] = cpSegmentShapeNew(self -> shape.base.body, a, b, self -> width / 2);
    }
}

static cpFloat moment(Line *self) {
    const double mass = cpBodyGetMass(self -> shape.base.body) / self -> shape.base.length;
    double moment = 0;

    FOR(size_t, self -> shape.base.length) {
        cpVect a = cpv(self -> shape.points[i][x], self -> shape.points[i][y]);
        cpVect b = cpv(self -> shape.points[i + 1][x], self -> shape.points[i + 1][y]);

        moment += cpMomentForSegment(mass, a, b, self -> width / 2);
    }

    return moment;
}

static void base(Line *self) {
    // if (!self -> base.shape) return;
    // cpVect *verts = vertices(self);

    // cpPolyShapeSetVerts(self -> base.shape, self -> vertex, verts, transform(self));
    // baseMoment((Base *) self);
    // free(verts);
}

static double top(Line *self) {
    return shapeTop((Shape *) self) + self -> width / 2;
}

static double bottom(Line *self) {
    return shapeBottom((Shape *) self) - self -> width / 2;
}

static double left(Line *self) {
    return shapeLeft((Shape *) self) - self -> width / 2;
}

static double right(Line *self) {
    return shapeRight((Shape *) self) + self -> width / 2;
}

static void parse(Line *self) {
    glBindVertexArray(self -> shape.vao);
    glBindBuffer(GL_ARRAY_BUFFER, self -> shape.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> shape.ibo);

    lineCreate(self -> shape.points, self -> shape.vertex, self -> width);
    glBindVertexArray(0);
}

static PyObject *Line_getWidth(Line *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> width);
}

static int Line_setWidth(Line *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> width = PyFloat_AsDouble(value);
    return ERR(self -> width) ? -1 : parse(self), base(self), 0;
}

static PyObject *Line_draw(Shape *self, PyObject *Py_UNUSED(ignored)) {
    baseMatrix((Base *) self, 1, 1);

    glBindVertexArray(self -> vao);
    glUniform1i(uniform[img], SHAPE);
    glDrawElements(GL_TRIANGLES, IDX(self -> vertex * 3 - 2), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    Py_RETURN_NONE;
}

static PyObject *Line_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Base *self = shapeNew(type);

    self -> new = (void *)(Base *) new;
    self -> moment = (cpFloat (*)(Base *)) moment;
    self -> top = (double (*)(Base *)) top;
    self -> bottom = (double (*)(Base *)) bottom;
    self -> left = (double (*)(Base *)) left;
    self -> right = (double (*)(Base *)) right;

    return (PyObject *) self;
}

static int Line_init(Line *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "width", "x", "y", "angle", "color", NULL};
    double angle = 0;
    self -> width = 2;

    PyObject *color = NULL;
    PyObject *points = NULL;
    baseInit((Base *) self);

    int state = PyArg_ParseTupleAndKeywords(
        args, kwds, "|OddddO", kwlist, &points, &self -> width, &self -> shape.base.pos[x],
        &self -> shape.base.pos[y], &angle, &color);

    if (!state || (color && vectorSet(color, self -> shape.base.color, 4)) || shapeParse((Shape *) self, points))
        return -1;

    if (!points) {
        self -> shape.vertex = 2;
        self -> shape.points = realloc(self -> shape.points, self -> shape.vertex * sizeof(vec2));

        self -> shape.points[0][x] = self -> shape.points[0][y] = -25;
        self -> shape.points[1][x] = self -> shape.points[1][y] = 25;
    }

    return baseStart((Base *) self, angle), parse(self), 0;
}

static PyGetSetDef LineGetSetters[] = {
    {"width", (getter) Line_getWidth, (setter) Line_setWidth, "thickness of the line", NULL},
    {NULL}
};

static PyMethodDef LineMethods[] = {
    {"draw", (PyCFunction) Line_draw, METH_NOARGS, "draw the line on the screen"},
    {NULL}
};

void lineCreate(poly base, size_t length, double width) {
    size_t verts = length * 3 - 2;
    size_t index = 0, start = 0, end = verts - 1;

    GLfloat *points = malloc(verts * 2 * sizeof(GLfloat));
    GLuint *indices = malloc(IDX(verts) * sizeof(GLuint));

    FOR(size_t, length) {
        vec e = base[i];

        vec2 prev = {
            i ? base[i - 1][x] : e[x],
            i ? base[i - 1][y] : e[y]
        };

        vec2 next = {
            i < length - 1 ? base[i + 1][x] : e[x],
            i < length - 1 ? base[i + 1][y] : e[y]
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
        vec2 pass = {hypot(hypot(ab[x], ab[y]), width / 2), hypot(hypot(bc[x], bc[y]),  width / 2)};

        normalize(ab);
        normalize(bc);

        vec2 tangent = {ab[x] + bc[x], ab[y] + bc[y]};
        vec2 point = {ab[x] - bc[x], ab[y] - bc[y]};

        normalize(tangent);
        normalize(point);

        vec2 miter = {-tangent[y], tangent[x]};
        vec2 normal = {-ab[y], ab[x]};

        const bool inside = miter[x] * point[x] + miter[y] * point[y] > 0;
        const size_t inner = start, outer = end;
        const char invert = inside ? 1 : -1;
        size_t a, b, c;

        if (inside) {
            a = start, b = end, c = end - 1;
            end -= i ? 2 : 1;
            start ++;
        }

        else {
            a = end, b = start, c = start + 1;
            start += i ? 2 : 1;
            end --;
        }

        const double dot = miter[x] * normal[x] + miter[y] * normal[y];
        const double line = width / 2 * invert;
        const double pro = line / dot * invert;
        const double min = MIN(pass[x], pass[y]);

        vec2 left = {normal[x] * line, normal[y] * line};
        vec2 right = {-(normal[x] - 2 * dot * miter[x]) * line, -(normal[y] - 2 * dot * miter[y]) * line};
        vec2 mid = {miter[x] * line / dot, miter[y] * line / dot};

        points[a * 2] = e[x] - (pro > min && min == pass[x] ? right[x] : pro > min && min == pass[y] ? left[x] : mid[x]);
        points[a * 2 + 1] = e[y] - (pro > min && min == pass[x] ? right[y] : pro > min && min == pass[y] ? left[y] : mid[y]);
        points[b * 2] = e[x] + left[x];
        points[b * 2 + 1] = e[y] + left[y];

        if (i) {
            indices[index] = indices[index + 5] = inner - 1;
            indices[index + 1] = inner;
            indices[index + 2] = indices[index + 3] = outer;
            indices[index + 4] = outer + 1;

            if (i < length - 1) {
                points[c * 2] = e[x] + right[x];
                points[c * 2 + 1] = e[y] + right[y];

                indices[index + 6] = a;
                indices[index + 7] = b;
                indices[index + 8] = c;
            }

            index += 9;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, verts * 2 * sizeof(GLfloat), points, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IDX(verts) * sizeof(GLuint), indices, GL_DYNAMIC_DRAW);

    free(points);
    free(indices);
}

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
    .tp_dealloc = (destructor) shapeDealloc,
    .tp_getset = LineGetSetters,
    .tp_methods = LineMethods
};