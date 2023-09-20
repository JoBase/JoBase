#include <main.h>

static void poly(Rectangle *self, Vec2 *points) {
    points[0].x = points[3].x = -self -> size.x / 2;
    points[0].y = points[1].y = self -> size.y / 2;
    points[1].x = points[2].x = self -> size.x / 2;
    points[2].y = points[3].y = -self -> size.y / 2;
}

static void unsafe(Rectangle *self) {
    Vec2 points[4];

    poly(self, points);
    cpPolyShapeSetVerts(self -> base.shape, 4, (cpVect *) points, Base_transform(&self -> base));
}

static cpShape *physics(Rectangle *self) {
    Vec2 points[4];

    poly(self, points);
    return cpPolyShapeNew(self -> base.body -> body, 4, (cpVect *) points, Base_transform(&self -> base), 0);
}

static Sides sides(Rectangle *self) {
    Vec2 points[4];

    poly(self, points);
    return Base_sides(&self -> base, points, 4);
}

static int update(Rectangle *self) {
    return Base_unsafe(&self -> base), 0;
}

static PyObject *Rectangle_get_width(Rectangle *self, void *closure) {
    return PyFloat_FromDouble(self -> size.x);
}

static int Rectangle_set_width(Rectangle *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(self -> size.x = PyFloat_AsDouble(value)) ? -1 : update(self);
}

static PyObject *Rectangle_get_height(Rectangle *self, void *closure) {
    return PyFloat_FromDouble(self -> size.y);
}

static int Rectangle_set_height(Rectangle *self, PyObject *value, void *closure) {
    DEL(value, "height")
    return ERR(self -> size.y = PyFloat_AsDouble(value)) ? -1 : update(self);
}

static Vector *Rectangle_get_size(Rectangle *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> size, 2, (set) update);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Rectangle_set_size(Rectangle *self, PyObject *value, void *closure) {
    DEL(value, "size")
    return Vector_set(value, (vec) &self -> size, 2) ? -1 : update(self);
}

static PyObject *Rectangle_draw(Rectangle *self, PyObject *args) {
    Rectangle_render(self, shape);
    Py_RETURN_NONE;
}

static int Rectangle_init(Rectangle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "width", "height", "angle", "color", NULL};

    PyObject *color = NULL;
    BaseType.tp_init((PyObject *) self, NULL, NULL);

    self -> size.x = 50;
    self -> size.y = 50;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO:Rectangle", kwlist, &self -> base.pos.x, &self -> base.pos.y,
        &self -> size.x, &self -> size.y, &self -> base.angle, &color))

    return Vector_set(color, (vec) &self -> base.color, 4);
}

Rectangle *Rectangle_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Rectangle *self = (Rectangle *) type -> tp_alloc(type, 0);

    if (self) {
        self -> base.sides = (Sides (*)(Base *)) sides;
        self -> base.physics = (cpShape *(*)(Base *)) physics;
        self -> base.unsafe = (void (*)(Base *)) unsafe;
    }

    return self;
}

void Rectangle_render(Rectangle *self, uint8_t type) {
    Base_matrix(&self -> base, type, self -> size.x, self -> size.y);

    glBindVertexArray(mesh);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Rectangle_poly(Rectangle *self, Vec2 *points) {
    Vec2 src[4];

    poly(self, src);
    Base_poly(&self -> base, src, points, 4);
}

static PyGetSetDef Rectangle_getset[] = {
    {"width", (getter) Rectangle_get_width, (setter) Rectangle_set_width, "width of the rectangle", NULL},
    {"height", (getter) Rectangle_get_height, (setter) Rectangle_set_height, "height of the rectangle", NULL},
    {"size", (getter) Rectangle_get_size, (setter) Rectangle_set_size, "dimentions of the rectangle", NULL},
    {NULL}
};

static PyMethodDef Rectangle_methods[] = {
    {"draw", (PyCFunction) Rectangle_draw, METH_NOARGS, "draw the rectangle on the screen"},
    {NULL}
};

PyTypeObject RectangleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Rectangle",
    .tp_doc = "draw rectangles on the screen",
    .tp_basicsize = sizeof(Rectangle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = (newfunc) Rectangle_new,
    .tp_init = (initproc) Rectangle_init,
    .tp_methods = Rectangle_methods,
    .tp_getset = Rectangle_getset
};