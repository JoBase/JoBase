#include <glad/glad.h>
#include <main.h>

static int Pivot_setFirstX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPivotJointGetAnchorA(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpPivotJointSetAnchorA(self -> joint, anchor), 0;
}

static int Pivot_setFirstY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPivotJointGetAnchorA(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpPivotJointSetAnchorA(self -> joint, anchor), 0;
}

static double Pivot_vecFirst(Joint *self, uint8_t index) {
    const cpVect anchor = cpPivotJointGetAnchorA(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Pivot_getFirst(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Pivot_vecFirst, 2);

    offset -> data[x].set = (setter) Pivot_setFirstX;
    offset -> data[y].set = (setter) Pivot_setFirstY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Pivot_setFirst(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpPivotJointGetAnchorA(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpPivotJointSetAnchorA(self -> joint, cpv(vect[x], vect[y])), 0;
}

static int Pivot_setLastX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPivotJointGetAnchorB(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpPivotJointSetAnchorB(self -> joint, anchor), 0;
}

static int Pivot_setLastY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpPivotJointGetAnchorB(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpPivotJointSetAnchorB(self -> joint, anchor), 0;
}

static double Pivot_vecLast(Joint *self, uint8_t index) {
    const cpVect anchor = cpPivotJointGetAnchorB(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Pivot_getLast(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Pivot_vecLast, 2);

    offset -> data[x].set = (setter) Pivot_setLastX;
    offset -> data[y].set = (setter) Pivot_setLastY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Pivot_setLast(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpPivotJointGetAnchorB(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpPivotJointSetAnchorB(self -> joint, cpv(vect[x], vect[y])), 0;
}

static PyObject *Pivot_draw(Joint *self, PyObject *Py_UNUSED(ignored)) {
    cpVect anchor = cpPivotJointGetAnchorA(self -> joint);
    vec a = self -> a -> pos;
    vec b = self -> b -> pos;

    vec2 point[] = {{anchor.x, anchor.y}};
    rotate(point, 1, cpBodyGetAngle(self -> a -> body), a);

    vec2 base[] = {{b[x], b[y]}, {(*point)[x], (*point)[y]}, {a[x], a[y]}};
    jointDraw(self, base, 3);

    Py_RETURN_NONE;
}

static PyObject *Pivot_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return jointNew(type, (cpConstraint *) cpPivotJointAlloc());
}

static int Pivot_init(Joint *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "width", "color", NULL};

    jointInit(self);
    PyObject *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|dO", kwlist, &BaseType, &self -> a,
        &BaseType, &self -> b, &self -> width, &color)) return -1;

    cpPivotJointInit((cpPivotJoint *) self -> joint, self -> a -> body, self -> b -> body, cpv(0, 0), cpv(0, 100));
    return jointStart(self, color);
}

static PyGetSetDef PivotGetSetters[] = {
    {"a", (getter) Pivot_getFirst, (setter) Pivot_setFirst, "offset of the first attached body", NULL},
    {"b", (getter) Pivot_getLast, (setter) Pivot_setLast, "offset of the last attached body", NULL},
    {NULL}
};

static PyMethodDef PivotMethods[] = {
    {"draw", (PyCFunction) Pivot_draw, METH_NOARGS, "draw the pivot joint on the screen"},
    {NULL}
};

PyTypeObject PivotType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Pivot",
    .tp_doc = "allows two bodies to rotate around a pivot",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = Pivot_new,
    .tp_init = (initproc) Pivot_init,
    .tp_getset = PivotGetSetters,
    .tp_methods = PivotMethods
};