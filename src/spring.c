#include <main.h>

static void unsafe(Spring *self) {
    cpDampedSpringSetAnchorA(self -> base.joint, Joint_rotate(self -> base.a, self -> start));
    cpDampedSpringSetAnchorB(self -> base.joint, Joint_rotate(self -> base.b, self -> end));
}

static int anchor(Spring *self) {
    if (self -> base.parent) unsafe(self);
    return 0;
}

static void create(Spring *self) {
    cpBody *a = self -> base.a -> body -> body;
    cpBody *b = self -> base.b -> body -> body;

    cpDampedSpringInit((cpDampedSpring *) self -> base.joint, a, b, cpv(0, 0), cpv(0, 0), self -> length, self -> stiffness, self -> damping);
}

static PyObject *Spring_get_length(Spring *self, void *closure) {
    return PyFloat_FromDouble(self -> length);
}

static int Spring_set_length(Spring *self, PyObject *value, void *closure) {
    DEL(value, "length")
    INIT(ERR(self -> length = PyFloat_AsDouble(value)))

    return self -> base.parent ? (cpDampedSpringSetRestLength(self -> base.joint, self -> length), 0) : 0;
}

static PyObject *Spring_get_stiffness(Spring *self, void *closure) {
    return PyFloat_FromDouble(self -> stiffness);
}

static int Spring_set_stiffness(Spring *self, PyObject *value, void *closure) {
    DEL(value, "stiffness")
    INIT(ERR(self -> stiffness = PyFloat_AsDouble(value)))

    return self -> base.parent ? (cpDampedSpringSetStiffness(self -> base.joint, self -> stiffness), 0) : 0;
}

static PyObject *Spring_get_damping(Spring *self, void *closure) {
    return PyFloat_FromDouble(self -> damping);
}

static int Spring_set_damping(Spring *self, PyObject *value, void *closure) {
    DEL(value, "damping")
    INIT(ERR(self -> damping = PyFloat_AsDouble(value)))

    return self -> base.parent ? (cpDampedSpringSetDamping(self -> base.joint, self -> damping), 0) : 0;
}

static Vector *Spring_get_start(Spring *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> start, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Spring_set_start(Spring *self, PyObject *value, void *closure) {
    DEL(value, "start")
    return Vector_set(value, (vec) &self -> start, 2) ? -1 : anchor(self);
}

static Vector *Spring_get_end(Spring *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> end, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Spring_set_end(Spring *self, PyObject *value, void *closure) {
    DEL(value, "end")
    return Vector_set(value, (vec) &self -> end, 2) ? -1 : anchor(self);
}

static PyObject *Spring_draw(Spring *self, PyObject *args) {
    if (Joint_active(&self -> base)) {
        const double length = sqrt(self -> length);
        const size_t verts = MAX(length * 2, 2);

        Vec2 a = Body_get(self -> base.a -> body, Joint_rotate(self -> base.a, self -> start));
        Vec2 b = Body_get(self -> base.b -> body, Joint_rotate(self -> base.b, self -> end));

        Vec2 vector = {b.x - a.x, b.y - a.y};
        Vec2 *base = malloc(verts * sizeof(Vec2));

        const double dist = hypot(vector.x, vector.y);
        const double space = dist / (verts - 1);

        vector.x /= dist;
        vector.y /= dist;

        for (size_t i = 0; i < verts; i ++) {
            if (i > 1 && i < verts - 2) {
                const double invert = i % 2 ? length : -length;

                base[i].x = a.x + vector.x * space * i - vector.y * invert;
                base[i].y = a.y + vector.y * space * i + vector.x * invert;
            }

            else {
                base[i].x = a.x + i * space * vector.x;
                base[i].y = a.y + i * space * vector.y;
            }
        }

        Joint_draw(&self -> base, base, verts);
        free(base);
    }

    Py_RETURN_NONE;
}

static Joint *Spring_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    return Joint_new(type, (joint) create, (move) unsafe, (cpConstraint *) cpDampedSpringAlloc());
}

static int Spring_init(Spring *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "length", "stiffness", "damping", "width", "color", NULL};

    PyObject *color = NULL;
    JointType.tp_init((PyObject *) self, NULL, NULL);

    self -> length = 0;
    self -> stiffness = 10;
    self -> damping = .5;

    self -> start.x = 0;
    self -> start.y = 0;
    self -> end.x = 0;
    self -> end.y = 0;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|ddddO:Spring", kwlist, &BaseType, &self -> base.a, &BaseType,
        &self -> base.b, &self -> length, &self -> stiffness, &self -> damping, &self -> base.width,
        &color) || Vector_set(color, (vec) &self -> base.color, 4))

    if (!self -> length) {
        const double a = self -> base.a -> pos.x - self -> base.b -> pos.x;
        const double b = self -> base.a -> pos.y - self -> base.b -> pos.y;

        self -> length = hypot(a, b);
    }

    return Joint_add(&self -> base), 0;
}

static PyGetSetDef Spring_getset[] = {
    {"start", (getter) Spring_get_start, (setter) Spring_set_start, "the offset of the spring relative to the first body", NULL},
    {"end", (getter) Spring_get_end, (setter) Spring_set_end, "the offset of the spring relative to the last body", NULL},
    {"length", (getter) Spring_get_length, (setter) Spring_set_length, "the resting length of the spring", NULL},
    {"stiffness", (getter) Spring_get_stiffness, (setter) Spring_set_stiffness, "the strength of the spring", NULL},
    {"damping", (getter) Spring_get_damping, (setter) Spring_set_damping, "the bounciness of the spring", NULL},
    {NULL}
};

static PyMethodDef Spring_methods[] = {
    {"draw", (PyCFunction) Spring_draw, METH_NOARGS, "draw the spring joint on the screen"},
    {NULL}
};

PyTypeObject SpringType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Spring",
    .tp_doc = "create a spring between two bodies",
    .tp_basicsize = sizeof(Spring),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = (newfunc) Spring_new,
    .tp_init = (initproc) Spring_init,
    .tp_methods = Spring_methods,
    .tp_getset = Spring_getset
};