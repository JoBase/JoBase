#include <main.h>

static int Pin_setFirstX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPinJointGetAnchorA(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpPinJointSetAnchorA(self -> joint, anchor), 0;
}

static int Pin_setFirstY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPinJointGetAnchorA(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpPinJointSetAnchorA(self -> joint, anchor), 0;
}

static double Pin_vecFirst(Joint *self, uint8_t index) {
    const cpVect anchor = cpPinJointGetAnchorA(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Pin_getFirst(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Pin_vecFirst, 2);

    offset -> data[x].set = (setter) Pin_setFirstX;
    offset -> data[y].set = (setter) Pin_setFirstY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Pin_setFirst(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpPinJointGetAnchorA(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpPinJointSetAnchorA(self -> joint, cpv(vect[x], vect[y])), 0;
}

static int Pin_setLastX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPinJointGetAnchorB(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpPinJointSetAnchorB(self -> joint, anchor), 0;
}

static int Pin_setLastY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPinJointGetAnchorB(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpPinJointSetAnchorB(self -> joint, anchor), 0;
}

static double Pin_vecLast(Joint *self, uint8_t index) {
    const cpVect anchor = cpPinJointGetAnchorB(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Pin_getLast(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Pin_vecLast, 2);

    offset -> data[x].set = (setter) Pin_setLastX;
    offset -> data[y].set = (setter) Pin_setLastY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Pin_setLast(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpPinJointGetAnchorB(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpPinJointSetAnchorB(self -> joint, cpv(vect[x], vect[y])), 0;
}

static PyObject *Pin_getLength(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cpPinJointGetDist(self -> joint));
}

static int Pin_setLength(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double length = PyFloat_AsDouble(value);
    return ERR(length) ? -1 : cpPinJointSetDist(self -> joint, length), 0;
}

static PyObject *Pin_draw(Joint *self, PyObject *Py_UNUSED(ignored)) {
    cpVect first = cpPinJointGetAnchorA(self -> joint);
    cpVect last = cpPinJointGetAnchorB(self -> joint);

    vec2 a[] = {{first.x, first.y}};
    vec2 b[] = {{last.x, last.y}};

    rotate(a, 1, cpBodyGetAngle(self -> a -> body), self -> a -> pos);
    rotate(b, 1, cpBodyGetAngle(self -> b -> body), self -> b -> pos);

    vec2 base[] = {{(*a)[x], (*a)[y]}, {(*b)[x], (*b)[y]}};
    jointDraw(self, base, 2);

    Py_RETURN_NONE;
}

static PyObject *Pin_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return jointNew(type, (cpConstraint *) cpPinJointAlloc());
}

static int Pin_init(Joint *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "length", "width", "color", NULL};

    jointInit(self);
    PyObject *color = NULL;
    double length = 0;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|ddO", kwlist, &BaseType, &self -> a, &BaseType,
        &self -> b, &length, &self -> width, &color)) return -1;

    if (!length) {
        const double a = self -> a -> pos[x] - self -> b -> pos[x];
        const double b = self -> a -> pos[y] - self -> b -> pos[y];

        length = hypot(a, b);
    }

    cpPinJointInit((cpPinJoint *) self -> joint, self -> a -> body, self -> b -> body, cpv(0, 0), cpv(0, 0));
    cpPinJointSetDist(self -> joint, length);
    return jointStart(self, color);
}

static PyGetSetDef PinGetSetters[] = {
    {"a", (getter) Pin_getFirst, (setter) Pin_setFirst, "offset of the first attached body", NULL},
    {"b", (getter) Pin_getLast, (setter) Pin_setLast, "offset of the last attached body", NULL},
    {"length", (getter) Pin_getLength, (setter) Pin_setLength, "distance between the two bodies", NULL},
    {NULL}
};

static PyMethodDef PinMethods[] = {
    {"draw", (PyCFunction) Pin_draw, METH_NOARGS, "draw the pin joint on the screen"},
    {NULL}
};

PyTypeObject PinType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Pin",
    .tp_doc = "keeps a set distance between two physics bodies",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = Pin_new,
    .tp_init = (initproc) Pin_init,
    .tp_getset = PinGetSetters,
    .tp_methods = PinMethods
};