#include <main.h>

static cpVect point(Line *self, size_t index) {
    cpVect rot = cpvforangle(self -> base.base.rotate * M_PI / 180);
    cpVect pos = cpv(self -> base.points[index].x * self -> base.base.scale.x, self -> base.points[index].y * self -> base.base.scale.y);

    return cpvadd(cpvrotate(pos, rot), cpv(self -> base.base.transform.x, self -> base.base.transform.y));
}

static void unsafe(Line *self) {
    const double radius = Base_radius(&self -> base.base, self -> width);
    cpShape *shape = self -> base.base.shape;

    for (size_t i = 0; i < self -> base.length - 1; i ++) {
        cpVect a = point(self, i);
        cpVect b = point(self, i + 1);
        cpShape *next = shape;

        if (i && !(next = cpShapeGetUserData(shape))) {
            next = cpSegmentShapeNew(self -> base.base.body -> body, a, b, radius);

            cpShapeSetUserData(shape, next);
            Base_shape(&self -> base.base, next);
        }

        else {
            cpSegmentShapeSetEndpoints(next, a, b);
            cpSegmentShapeSetRadius(next, radius);
        }

        shape = next;
    }

    Shape_reduce(&self -> base, shape);
}

static cpShape *physics(Line *self) {
    const double radius = Base_radius(&self -> base.base, self -> width);
    cpShape *shape = NULL;

    for (size_t i = 0; i < self -> base.length - 1; i ++) {
        cpVect a = point(self, i);
        cpVect b = point(self, i + 1);
        cpShape *array = shape;

        shape = cpSegmentShapeNew(self -> base.base.body -> body, a, b, radius);
        cpShapeSetUserData(shape, array);
    }

    return shape;
}

static Vec2 normalize(double x, double y) {
    const double length = hypot(x, y);
    Vec2 value = {x / length, y / length};

    return value;
}

static int create(Line *self) {
    if (self -> base.length < 2) {
        PyErr_SetString(PyExc_ValueError, "line must contain a minimum of 2 points");
        return -1;
    }

    glBindVertexArray(self -> base.vao);
    glBindBuffer(GL_ARRAY_BUFFER, self -> base.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> base.ibo);

    Line_create(self -> base.points, self -> base.length, self -> width);
    glBindVertexArray(0);

    return Base_unsafe(&self -> base.base), 0;
}

static Sides sides(Line *self) {
    Sides sides = Shape_sides(&self -> base);

    sides.top += self -> width / 2;
    sides.bottom -= self -> width / 2;
    sides.left -= self -> width / 2;
    sides.right += self -> width / 2;

    return sides;
}

static PyObject *Line_get_width(Line *self, void *closure) {
    return PyFloat_FromDouble(self -> width);
}

static int Line_set_width(Line *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(self -> width = PyFloat_AsDouble(value)) ? -1 : create(self);
}

static PyObject *Line_draw(Shape *self, PyObject *args) {
    return Shape_render(self, self -> length * 3 - 2);
}

static Shape *Line_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Shape *self = Shape_new(type, args, kwds);

    if (self) {
        self -> base.sides = (Sides (*)(Base *)) sides;
        self -> base.physics = (cpShape *(*)(Base *)) physics;
        self -> base.unsafe = (void (*)(Base *)) unsafe;
        self -> reset = (reset) create;
    }

    return self;
}

static int Line_init(Line *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"points", "width", "x", "y", "angle", "color", NULL};

    PyObject *color = NULL;
    PyObject *points = NULL;

    BaseType.tp_init((PyObject *) self, NULL, NULL);
    self -> width = 2;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|OddddO:Line", kwlist, &points, &self -> width,
        &self -> base.base.pos.x, &self -> base.base.pos.y, &self -> base.base.angle,
        &color) || Points_set(&self -> base, points) || Vector_set(color, (vec) &self -> base.base.color, 4))

    if (!points) {
        self -> base.length = 2;
        self -> base.points = realloc(self -> base.points, self -> base.length * sizeof(Vec2));

        self -> base.points[0].x = self -> base.points[0].y = -25;
        self -> base.points[1].x = self -> base.points[1].y = 25;
    }

    return create(self);
}

void Line_create(Vec2 *points, size_t base, double width) {
    const size_t verts = base * 3 - 2;
    size_t index = 0, start = 0, end = verts - 1, length, size;

    GLfloat *vertices = malloc(length = verts * 2 * sizeof(GLfloat));
    GLuint *indices = malloc(size = (verts - 2) * 3 * sizeof(GLuint));

    for (size_t i = 0; i < base; i ++) {
        Vec2 e = points[i];

        Vec2 prev = {
            i ? points[i - 1].x : e.x,
            i ? points[i - 1].y : e.y
        };

        Vec2 next = {
            i < base - 1 ? points[i + 1].x : e.x,
            i < base - 1 ? points[i + 1].y : e.y
        };

        if (e.x == prev.x && e.y == prev.y) {
            Vec2 vect = normalize(e.x - next.x, e.y - next.y);

            prev.x += vect.x;
            prev.y += vect.y;
        }

        if (e.x == next.x && e.y == next.y) {
            Vec2 vect = normalize(e.x - prev.x, e.y - prev.y);

            next.x += vect.x;
            next.y += vect.y;
        }

        Vec2 ab = {e.x - prev.x, e.y - prev.y};
        Vec2 bc = {next.x - e.x, next.y - e.y};

        Vec2 pass = {
            hypot(hypot(ab.x, ab.y), width / 2),
            hypot(hypot(bc.x, bc.y), width / 2)
        };

        ab = normalize(ab.x, ab.y);
        bc = normalize(bc.x, bc.y);

        Vec2 tangent = normalize(ab.x + bc.x, ab.y + bc.y);
        Vec2 point = normalize(ab.x - bc.x, ab.y - bc.y);
        Vec2 miter = {-tangent.y, tangent.x};
        Vec2 normal = {-ab.y, ab.x};

        const bool inside = miter.x * point.x + miter.y * point.y > 0;
        const char invert = inside ? 1 : -1;
        size_t inner = start, outer = end, a, b, c;

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

        const double dot = miter.x * normal.x + miter.y * normal.y;
        const double line = width / 2 * invert;
        const double pro = line / dot * invert;
        const double min = MIN(pass.x, pass.y);

        Vec2 left = {normal.x * line, normal.y * line};
        Vec2 right = {-(normal.x - 2 * dot * miter.x) * line, -(normal.y - 2 * dot * miter.y) * line};
        Vec2 mid = {miter.x * line / dot, miter.y * line / dot};

        vertices[a * 2] = e.x - (pro > min && min == pass.x ? right.x : pro > min && min == pass.y ? left.x : mid.x);
        vertices[a * 2 + 1] = e.y - (pro > min && min == pass.x ? right.y : pro > min && min == pass.y ? left.y : mid.y);
        vertices[b * 2] = e.x + left.x;
        vertices[b * 2 + 1] = e.y + left.y;

        if (i) {
            indices[index] = indices[index + 5] = inner - 1;
            indices[index + 1] = inner;
            indices[index + 2] = indices[index + 3] = outer;
            indices[index + 4] = outer + 1;

            if (i < base - 1) {
                vertices[c * 2] = e.x + right.x;
                vertices[c * 2 + 1] = e.y + right.y;

                indices[index + 6] = a;
                indices[index + 7] = b;
                indices[index + 8] = c;
            }

            index += 9;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, length, vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_DYNAMIC_DRAW);

    free(vertices);
    free(indices);
}

static PyGetSetDef Line_getset[] = {
    {"width", (getter) Line_get_width, (setter) Line_set_width, "thickness of the line", NULL},
    {NULL}
};

static PyMethodDef Line_methods[] = {
    {"draw", (PyCFunction) Line_draw, METH_NOARGS, "draw the line on the screen"},
    {NULL}
};

PyTypeObject LineType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Line",
    .tp_doc = "draw lines on the screen",
    .tp_basicsize = sizeof(Line),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &ShapeType,
    .tp_new = (newfunc) Line_new,
    .tp_init = (initproc) Line_init,
    .tp_dealloc = (destructor) Shape_dealloc,
    .tp_methods = Line_methods,
    .tp_getset = Line_getset
};