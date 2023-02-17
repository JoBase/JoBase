#include <glad/glad.h>
#include <main.h>

static int Spring_setFirstX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpDampedSpringGetAnchorA(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpDampedSpringSetAnchorA(self -> joint, anchor), 0;
}

static int Spring_setFirstY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpDampedSpringGetAnchorA(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpDampedSpringSetAnchorA(self -> joint, anchor), 0;
}

static double Spring_vecFirst(Joint *self, uint8_t index) {
    const cpVect anchor = cpDampedSpringGetAnchorA(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Spring_getFirst(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Spring_vecFirst, 2);

    offset -> data[x].set = (setter) Spring_setFirstX;
    offset -> data[y].set = (setter) Spring_setFirstY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Spring_setFirst(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpDampedSpringGetAnchorA(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpDampedSpringSetAnchorA(self -> joint, cpv(vect[x], vect[y])), 0;
}

static int Spring_setLastX(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpDampedSpringGetAnchorB(self -> joint);
    return ERR(anchor.x = PyFloat_AsDouble(value)) ? -1 : cpDampedSpringSetAnchorB(self -> joint, anchor), 0;
}

static int Spring_setLastY(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    cpVect anchor = cpDampedSpringGetAnchorB(self -> joint);
    return ERR(anchor.y = PyFloat_AsDouble(value)) ? -1 : cpDampedSpringSetAnchorB(self -> joint, anchor), 0;
}

static double Spring_vecLast(Joint *self, uint8_t index) {
    const cpVect anchor = cpDampedSpringGetAnchorB(self -> joint);
    const vec2 vector = {anchor.x, anchor.y};
    return vector[index];
}

static PyObject *Spring_getLast(Joint *self, void *Py_UNUSED(closure)) {
    Vector *offset = vectorNew((PyObject *) self, (Getter) Spring_vecLast, 2);

    offset -> data[x].set = (setter) Spring_setLastX;
    offset -> data[y].set = (setter) Spring_setLastY;
    offset -> data[x].name = "x";
    offset -> data[y].name = "y";

    return (PyObject *) offset;
}

static int Spring_setLast(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    const cpVect anchor = cpDampedSpringGetAnchorB(self -> joint);
    vec2 vect = {anchor.x, anchor.y};
    return vectorSet(value, vect, 2) ? -1 : cpDampedSpringSetAnchorB(self -> joint, cpv(vect[x], vect[y])), 0;
}

static PyObject *Spring_getLength(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cpDampedSpringGetRestLength(self -> joint));
}

static int Spring_setLength(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double length = PyFloat_AsDouble(value);
    return ERR(length) ? -1 : cpDampedSpringSetRestLength(self -> joint, length), 0;
}

static PyObject *Spring_getStiffness(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cpDampedSpringGetStiffness(self -> joint));
}

static int Spring_setStiffness(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double length = PyFloat_AsDouble(value);
    return ERR(length) ? -1 : cpDampedSpringSetStiffness(self -> joint, length), 0;
}

static PyObject *Spring_getDamping(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cpDampedSpringGetDamping(self -> joint));
}

static int Spring_setDamping(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double length = PyFloat_AsDouble(value);
    return ERR(length) ? -1 : cpDampedSpringSetDamping(self -> joint, length), 0;
}

static PyObject *Spring_draw(Joint *self, PyObject *Py_UNUSED(ignored)) {
    const double length = sqrt(cpDampedSpringGetRestLength(self -> joint));
    const size_t verts = MAX(length, 2);

    cpVect first = cpDampedSpringGetAnchorA(self -> joint);
    cpVect last = cpDampedSpringGetAnchorB(self -> joint);

    vec2 a[] = {{first.x, first.y}};
    vec2 b[] = {{last.x, last.y}};

    rotate(a, 1, cpBodyGetAngle(self -> a -> body), self -> a -> pos);
    rotate(b, 1, cpBodyGetAngle(self -> b -> body), self -> b -> pos);

    vec2 vector = {(*b)[x] - (*a)[x], (*b)[y] - (*a)[y]};
    poly base = malloc(verts * sizeof(vec2));

    const double dist = hypot(vector[x], vector[y]);
    const double space = dist / (verts - 1);

    vector[x] /= dist;
    vector[y] /= dist;

    FOR(size_t, verts) {
        if (i > 1 && i < verts - 2) {
            const double invert = i % 2 ? length : -length;

            base[i][x] = (*a)[x] + vector[x] * space * i - vector[y] * invert;
            base[i][y] = (*a)[y] + vector[y] * space * i + vector[x] * invert;
        }

        else {
            base[i][x] = (*a)[x] + i * space * vector[x];
            base[i][y] = (*a)[y] + i * space * vector[y];
        }
    }

    jointDraw(self, base, verts);
    free(base);

    Py_RETURN_NONE;
}

static PyObject *Spring_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return jointNew(type, (cpConstraint *) cpDampedSpringAlloc());
}

static int Spring_init(Joint *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "length", "stiffness", "damping", "width", "color", NULL};

    jointInit(self);
    PyObject *color = NULL;
    double length = 0, stiffness = 10, damping = .5;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|ddddO", kwlist, &BaseType, &self -> a,
        &BaseType, &self -> b, &length, &stiffness, &damping,
        &self -> width, &color)) return -1;

    if (!length) {
        const double a = self -> a -> pos[x] - self -> b -> pos[x];
        const double b = self -> a -> pos[y] - self -> b -> pos[y];

        length = hypot(a, b);
    }

    cpDampedSpringInit((cpDampedSpring *) self -> joint, self -> a -> body, self -> b -> body, cpv(0, 0), cpv(0, 0), length, stiffness, damping);
    return jointStart(self, color);
}

static PyGetSetDef SpringGetSetters[] = {
    {"a", (getter) Spring_getFirst, (setter) Spring_setFirst, "offset of the first attached body", NULL},
    {"b", (getter) Spring_getLast, (setter) Spring_setLast, "offset of the last attached body", NULL},
    {"length", (getter) Spring_getLength, (setter) Spring_setLength, "resting length of the spring", NULL},
    {"stiffness", (getter) Spring_getStiffness, (setter) Spring_setStiffness, "strength of the spring", NULL},
    {"damping", (getter) Spring_getDamping, (setter) Spring_setDamping, "force momentum of the spring", NULL},
    {NULL}
};

static PyMethodDef SpringMethods[] = {
    {"draw", (PyCFunction) Spring_draw, METH_NOARGS, "draw the spring joint on the screen"},
    {NULL}
};

PyTypeObject SpringType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Spring",
    .tp_doc = "constrain two bodies with a spring",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = Spring_new,
    .tp_init = (initproc) Spring_init,
    .tp_getset = SpringGetSetters,
    .tp_methods = SpringMethods
};