#include <main.h>

static int Groove_setStartX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect start = cpGrooveJointGetGrooveA(self -> joint);
    return ERR(start.x = PyFloat_AsDouble(value)) ? -1 : cpGrooveJointSetGrooveA(self -> joint, start), 0;
}

static int Groove_setStartY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect start = cpGrooveJointGetGrooveA(self -> joint);
    return ERR(start.y = PyFloat_AsDouble(value)) ? -1 : cpGrooveJointSetGrooveA(self -> joint, start), 0;
}

static double Groove_vecStart(Joint *self, uint8_t index) {
    const cpVect start = cpGrooveJointGetGrooveA(self -> joint);
    const vec2 vector = {start.x, start.y};
    return vector[index];
}

static PyObject *Groove_getStart(Joint *self, void *Py_UNUSED(closure)) {
    Vector *start = vectorNew((PyObject *) self, (Getter) Groove_vecStart, 2);

    start -> data[x].set = (setter) Groove_setStartX;
    start -> data[y].set = (setter) Groove_setStartY;
    start -> data[x].name = "x";
    start -> data[y].name = "y";

    return (PyObject *) start;
}

static int Groove_setStart(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect start = cpGrooveJointGetGrooveA(self -> joint);
    vec2 vect = {start.x, start.y};
    return vectorSet(value, vect, 2) ? -1 : cpGrooveJointSetGrooveA(self -> joint, cpv(vect[x], vect[y])), 0;
}

static int Groove_setEndX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect end = cpGrooveJointGetGrooveB(self -> joint);
    return ERR(end.x = PyFloat_AsDouble(value)) ? -1 : cpGrooveJointSetGrooveB(self -> joint, end), 0;
}

static int Groove_setEndY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect end = cpGrooveJointGetGrooveB(self -> joint);
    return ERR(end.y = PyFloat_AsDouble(value)) ? -1 : cpGrooveJointSetGrooveB(self -> joint, end), 0;
}

static double Groove_vecEnd(Joint *self, uint8_t index) {
    const cpVect end = cpGrooveJointGetGrooveB(self -> joint);
    const vec2 vector = {end.x, end.y};
    return vector[index];
}

static PyObject *Groove_getEnd(Joint *self, void *Py_UNUSED(closure)) {
    Vector *end = vectorNew((PyObject *) self, (Getter) Groove_vecEnd, 2);

    end -> data[x].set = (setter) Groove_setEndX;
    end -> data[y].set = (setter) Groove_setEndY;
    end -> data[x].name = "x";
    end -> data[y].name = "y";

    return (PyObject *) end;
}

static int Groove_setEnd(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect end = cpGrooveJointGetGrooveB(self -> joint);
    vec2 vect = {end.x, end.y};
    return vectorSet(value, vect, 2) ? -1 : cpGrooveJointSetGrooveB(self -> joint, cpv(vect[x], vect[y])), 0;
}

static int Groove_setLastX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpGrooveJointGetAnchorB(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpGrooveJointSetAnchorB(self -> joint, anchor), 0;
}

static int Groove_setLastY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpGrooveJointGetAnchorB(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpGrooveJointSetAnchorB(self -> joint, anchor), 0;
}

static double Groove_vecLast(Joint *self, uint8_t index) {
    const cpVect anchor = cpGrooveJointGetAnchorB(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Groove_getLast(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Groove_vecLast, 2);

    offset -> data[x].set = (setter) Groove_setLastX;
    offset -> data[y].set = (setter) Groove_setLastY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Groove_setLast(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpGrooveJointGetAnchorB(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpGrooveJointSetAnchorB(self -> joint, cpv(vect[x], vect[y])), 0;
}

static PyObject *Groove_draw(Joint *self, PyObject *Py_UNUSED(ignored)) {
    cpVect first = cpGrooveJointGetGrooveA(self -> joint);
    cpVect last = cpGrooveJointGetGrooveB(self -> joint);

    vec2 a[] = {{first.x, first.y}};
    vec2 b[] = {{last.x, last.y}};

    rotate(a, 1, cpBodyGetAngle(self -> a -> body), self -> a -> pos);
    rotate(b, 1, cpBodyGetAngle(self -> a -> body), self -> a -> pos);

    vec2 base[] = {{(*a)[x], (*a)[y]}, {(*b)[x], (*b)[y]}};
    jointDraw(self, base, 2);

    Py_RETURN_NONE;
}

static PyObject *Groove_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return jointNew(type, (cpConstraint *) cpGrooveJointAlloc());
}

static int Groove_init(Joint *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "start", "end", "width", "color", NULL};
    jointInit(self);

    PyObject *color = NULL;
    PyObject *start = NULL;
    PyObject *end = NULL;

    vec2 first = {50, 0};
    vec2 last = {100, 0};

    int state = PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|OOdO", kwlist, &BaseType, &self -> a, &BaseType,
        &self -> b, &start, &end, &self -> width, &color);

    if (!state || (start && vectorSet(start, first, 2)) || (end && vectorSet(end, last, 2)))
        return -1;

    cpGrooveJointInit((cpGrooveJoint *) self -> joint, self -> a -> body, self -> b -> body, cpv(first[x], first[y]), cpv(last[x], last[y]), cpv(0, 0));
    return jointStart(self, color);
}

static PyGetSetDef GrooveGetSetters[] = {
    {"start", (getter) Groove_getStart, (setter) Groove_setStart, "start of the groove relative to the first body", NULL},
    {"end", (getter) Groove_getEnd, (setter) Groove_setEnd, "end of the groove relative to the first body", NULL},
    {"b", (getter) Groove_getLast, (setter) Groove_setLast, "offset of the last attached body", NULL},
    {NULL}
};

static PyMethodDef GrooveMethods[] = {
    {"draw", (PyCFunction) Groove_draw, METH_NOARGS, "draw the groove joint on the screen"},
    {NULL}
};

PyTypeObject GrooveType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Groove",
    .tp_doc = "keeps a set distance between two physics bodies",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = Groove_new,
    .tp_init = (initproc) Groove_init,
    .tp_getset = GrooveGetSetters,
    .tp_methods = GrooveMethods
};