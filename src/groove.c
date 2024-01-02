#include <main.h>

static void unsafe(Groove *self) {
    cpGrooveJointSetGrooveA(self -> base.joint, Joint_rotate(self -> base.a, self -> start));
    cpGrooveJointSetGrooveB(self -> base.joint, Joint_rotate(self -> base.a, self -> groove));
    cpGrooveJointSetAnchorB(self -> base.joint, Joint_rotate(self -> base.b, self -> end));
}

static int anchor(Groove *self) {
    if (self -> base.parent) unsafe(self);
    return 0;
}

static void create(Joint *self) {
    cpGrooveJointInit((cpGrooveJoint *) self -> joint, self -> a -> body -> body, self -> b -> body -> body, cpv(0, 0), cpv(0, 0), cpv(0, 0));
}

static Vector *Groove_get_start(Groove *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> start, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Groove_set_start(Groove *self, PyObject *value, void *closure) {
    DEL(value, "start")
    return Vector_set(value, (vec) &self -> start, 2) ? -1 : anchor(self);
}

static Vector *Groove_get_groove(Groove *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> groove, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Groove_set_groove(Groove *self, PyObject *value, void *closure) {
    DEL(value, "groove")
    return Vector_set(value, (vec) &self -> groove, 2) ? -1 : anchor(self);
}

static Vector *Groove_get_end(Groove *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> end, 2, (set) anchor);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Groove_set_end(Groove *self, PyObject *value, void *closure) {
    DEL(value, "end")
    return Vector_set(value, (vec) &self -> end, 2) ? -1 : anchor(self);
}

static PyObject *Groove_draw(Groove *self, PyObject *args) {
    if (Joint_active(&self -> base)) {
        Vec2 base[] = {
            Body_get(self -> base.a -> body, Joint_rotate(self -> base.a, self -> start)),
            Body_get(self -> base.a -> body, Joint_rotate(self -> base.a, self -> groove))
        };

        Joint_draw(&self -> base, base, 2);
    }

    Py_RETURN_NONE;
}

static Joint *Groove_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    return Joint_new(type, (joint) create, (move) unsafe, (cpConstraint *) cpGrooveJointAlloc());
}

static int Groove_init(Groove *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "start", "groove", "width", "color", NULL};

    PyObject *color = NULL;
    PyObject *start = NULL;
    PyObject *groove = NULL;
    JointType.tp_init((PyObject *) self, NULL, NULL);

    self -> start.x = 0;
    self -> start.y = 0;
    self -> groove.x = 50;
    self -> groove.y = 50;
    self -> end.x = 0;
    self -> end.y = 0;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|OOdO:Groove", kwlist, &BaseType, &self -> base.a,
        &BaseType, &self -> base.b, &start, &groove, &self -> base.width,
        &color) || Vector_set(color, (vec) &self -> base.color, 4) || Vector_set(start, (vec) &self -> start, 2) || Vector_set(groove, (vec) &self -> groove, 2))

    return Joint_add(&self -> base), 0;
}

static PyGetSetDef Groove_getset[] = {
    {"start", (getter) Groove_get_start, (setter) Groove_set_start, "the offset of the groove relative to the first body", NULL},
    {"groove", (getter) Groove_get_groove, (setter) Groove_set_groove, "the offset of the joint relative to the first body", NULL},
    {"end", (getter) Groove_get_end, (setter) Groove_set_end, "the offset of the groove on the last body", NULL},
    {NULL}
};

static PyMethodDef Groove_methods[] = {
    {"draw", (PyCFunction) Groove_draw, METH_NOARGS, "draw the groove joint on the screen"},
    {NULL}
};

PyTypeObject GrooveType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Groove",
    .tp_doc = "allow a body to slide along a groove",
    .tp_basicsize = sizeof(Groove),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = (newfunc) Groove_new,
    .tp_init = (initproc) Groove_init,
    .tp_methods = Groove_methods,
    .tp_getset = Groove_getset
};