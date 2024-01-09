#include <main.h>

static void unsafe(Pin *self) {
    cpPinJointSetAnchorA(self -> base.joint, Joint_rotate(self -> base.a, self -> start));
    cpPinJointSetAnchorB(self -> base.joint, Joint_rotate(self -> base.b, self -> end));
}

static int anchor(Pin *self) {
    if (self -> base.parent) unsafe(self);
    return 0;
}

static void create(Pin *self) {
    cpPinJointInit((cpPinJoint *) self -> base.joint, self -> base.a -> body -> body, self -> base.b -> body -> body, cpv(0, 0), cpv(0, 0));
    cpPinJointSetDist(self -> base.joint, self -> length);
}

static PyObject *Pin_get_length(Pin *self, void *closure) {
    return PyFloat_FromDouble(self -> length);
}

static int Pin_set_length(Pin *self, PyObject *value, void *closure) {
    DEL(value, "length")
    INIT(ERR(self -> length = PyFloat_AsDouble(value)))

    return self -> base.parent ? (cpPinJointSetDist(self -> base.joint, self -> length), 0) : 0;
}

static Vector *Pin_get_start(Pin *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> start, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Pin_set_start(Pin *self, PyObject *value, void *closure) {
    DEL(value, "start")
    return Vector_set(value, (vec) &self -> start, 2) ? -1 : anchor(self);
}

static Vector *Pin_get_end(Pin *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> end, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Pin_set_end(Pin *self, PyObject *value, void *closure) {
    DEL(value, "end")
    return Vector_set(value, (vec) &self -> end, 2) ? -1 : anchor(self);
}

static PyObject *Pin_draw(Pin *self, PyObject *args) {
    if (Joint_active(&self -> base)) {
        Vec2 base[] = {
            Body_get(self -> base.a -> body, Joint_rotate(self -> base.a, self -> start)),
            Body_get(self -> base.b -> body, Joint_rotate(self -> base.b, self -> end))
        };

        Joint_draw(&self -> base, base, 2);
    }

    Py_RETURN_NONE;
}

static Joint *Pin_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    return Joint_new(type, (joint) create, (move) unsafe, (cpConstraint *) cpPinJointAlloc());
}

static int Pin_init(Pin *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "length", "width", "color", NULL};

    PyObject *color = NULL;
    JointType.tp_init((PyObject *) self, NULL, NULL);

    self -> length = 0;
    self -> start.x = 0;
    self -> start.y = 0;
    self -> end.x = 0;
    self -> end.y = 0;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|ddO:Pin", kwlist, &BaseType, &self -> base.a,
        &BaseType, &self -> base.b, &self -> length, &self -> base.width,
        &color) || Vector_set(color, (vec) &self -> base.color, 4))

    if (!self -> length) {
        const double a = self -> base.a -> pos.x - self -> base.b -> pos.x;
        const double b = self -> base.a -> pos.y - self -> base.b -> pos.y;

        self -> length = hypot(a, b);
    }

    return Joint_add(&self -> base), 0;
}

static PyGetSetDef Pin_getset[] = {
    {"start", (getter) Pin_get_start, (setter) Pin_set_start, "the offset of the pin relative to the first body", NULL},
    {"end", (getter) Pin_get_end, (setter) Pin_set_end, "the offset of the pin relative to the last body", NULL},
    {"length", (getter) Pin_get_length, (setter) Pin_set_length, "the distance between the two bodies", NULL},
    {NULL}
};

static PyMethodDef Pin_methods[] = {
    {"draw", (PyCFunction) Pin_draw, METH_NOARGS, "draw the pin joint on the screen"},
    {NULL}
};

PyTypeObject PinType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Pin",
    .tp_doc = "constrain the distance between two bodies",
    .tp_basicsize = sizeof(Pin),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = (newfunc) Pin_new,
    .tp_init = (initproc) Pin_init,
    .tp_methods = Pin_methods,
    .tp_getset = Pin_getset
};