#define _USE_MATH_DEFINES
#include <main.h>

static void delete(Physics *self, size_t index) {
    Base *object = self -> data[index];

    cpSpaceRemoveBody(self -> space, object -> body);
    cpSpaceRemoveShape(self -> space, object -> shape);
    cpBodyFree(object -> body);
    cpShapeFree(object -> shape);

    object -> body = NULL;
    object -> shape = NULL;
    Py_DECREF(object);
}

static Py_ssize_t Physics_len(Physics *self) {
    return self -> length;
}

static PyObject *Physics_item(Physics *self, Py_ssize_t index) {
    if (index >= (Py_ssize_t) self -> length) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }

    Py_INCREF(self -> data[index]);
    return (PyObject *) self -> data[index];
}

static PySequenceMethods PhysicsSequenceMethods = {
    .sq_length = (lenfunc) Physics_len,
    .sq_item = (ssizeargfunc) Physics_item
};

static int Physics_setGravityX(Physics *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double gravity = PyFloat_AsDouble(value);
    if (ERR(gravity)) return -1;

    cpSpaceSetGravity(self -> space, cpv(gravity, cpSpaceGetGravity(self -> space).y));
    return 0;
}

static int Physics_setGravityY(Physics *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double gravity = PyFloat_AsDouble(value);
    if (ERR(gravity)) return -1;

    cpSpaceSetGravity(self -> space, cpv(cpSpaceGetGravity(self -> space).x, gravity));
    return 0;
}

static vec Physics_vecGravity(Physics *self) {
    static vec2 gravity;
    cpVect vect = cpSpaceGetGravity(self -> space);

    gravity[x] = vect.x;
    gravity[y] = vect.y;
    return gravity;
}

static PyObject *Physics_getGravity(Physics *self, void *Py_UNUSED(closure)) {
    Vector *gravity = vectorNew((PyObject *) self, (Getter) Physics_vecGravity, 2);

    gravity -> data[x].set = (setter) Physics_setGravityX;
    gravity -> data[y].set = (setter) Physics_setGravityY;
    gravity -> data[x].name = "x";
    gravity -> data[y].name = "y";

    return (PyObject *) gravity;
}

static int Physics_setGravity(Physics *self, PyObject *value, void *Py_UNUSED(closure)) {
    vec vect = Physics_vecGravity(self);

    if (vectorSet(value, vect, 2)) return -1;
    return cpSpaceSetGravity(self -> space, cpv(vect[x], vect[y])), 0;
}

static PyObject *Physics_add(Physics *self, PyObject *args) {
    Base *base;

    if (!PyArg_ParseTuple(args, "O!", &BaseType, &base))
        return NULL;

    if (base -> shape) {
        PyErr_SetString(PyExc_ValueError, "already added to a physics engine");
        return NULL;
    }

    if (base -> type == DYNAMIC)
        base -> body = cpBodyNew(base -> mass, base -> rotate ? base -> moment(base) : INFINITY);

    else base -> body = cpBodyNewKinematic();
    cpBodySetAngle(base -> body, base -> angle * M_PI / 180);
    cpBodySetPosition(base -> body, cpv(base -> pos[x], base -> pos[y]));
    cpBodySetVelocity(base -> body, cpv(base -> vel[x], base -> vel[y]));
    cpBodySetAngularVelocity(base -> body, base -> angular * M_PI / 180);

    base -> new(base);
    cpShapeSetElasticity(base -> shape, base -> elasticity);
    cpShapeSetFriction(base -> shape, base -> friction);

    cpSpaceAddBody(self -> space, base -> body);
    cpSpaceAddShape(self -> space, base -> shape);

    self -> data = realloc(self -> data, sizeof(Base *) * (self -> length + 1));
    self -> data[self -> length] = base;
    self -> length ++;

    Py_INCREF(base);
    Py_RETURN_NONE;
}

static PyObject *Physics_remove(Physics *self, PyObject *args) {
    Base *other;

    if (!PyArg_ParseTuple(args, "O!", &BaseType, &other))
        return NULL;

    FOR(size_t, self -> length)
        if (self -> data[i] == other) {
            delete(self, i);
            self -> length --;

            for (size_t j = i; j < self -> length; j ++)
                self -> data[j] = self -> data[j + 1];

            self -> data = realloc(self -> data, sizeof(Base *) * self -> length);
            Py_RETURN_NONE;
        }

    PyErr_SetString(PyExc_ValueError, "can't remove because it doesn't exist in physics engine");
    return NULL;
}

static PyObject *Physics_update(Physics *self, PyObject *Py_UNUSED(ignored)) {
    cpSpaceStep(self -> space, 1. / 60);

    for (size_t i = 0; i < self -> length; i ++) {
        cpVect pos = cpBodyGetPosition(self -> data[i] -> body);
        cpVect vel = cpBodyGetVelocity(self -> data[i] -> body);
        cpFloat angle = cpBodyGetAngle(self -> data[i] -> body);
        cpFloat angular = cpBodyGetAngularVelocity(self -> data[i] -> body);

        self -> data[i] -> pos[x] = pos.x;
        self -> data[i] -> pos[y] = pos.y;
        self -> data[i] -> vel[x] = vel.x;
        self -> data[i] -> vel[y] = vel.y;
        self -> data[i] -> angle = angle * 180 / M_PI;
        self -> data[i] -> angular = angular * 180 / M_PI;
    }
    
    Py_RETURN_NONE;
}

static PyObject *Physics_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Physics *self = (Physics *) type -> tp_alloc(type, 0);

    self -> data = realloc(self -> data, 0);
    self -> space = cpSpaceNew();

    return (PyObject *) self;
}

static int Physics_init(Physics *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"gravity_x", "gravity_y", NULL};
    cpVect vector = {0, -500};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dd", kwlist, &vector.x, &vector.y))
        return -1;

    FOR(size_t, self -> length) delete(self, i);
    self -> data = realloc(self -> data, 0);
    self -> length = 0;

    return cpSpaceSetGravity(self -> space, vector), 0;
}

static void Physics_dealloc(Physics *self) {
    FOR(size_t, self -> length) delete(self, i);
    cpSpaceFree(self -> space);
    free(self -> data);

    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyGetSetDef PhysicsGetSetters[] = {
    {"gravity", (getter) Physics_getGravity, (setter) Physics_setGravity, "the gravity of the physics engine", NULL},
    {NULL}
};

static PyMethodDef PhysicsMethods[] = {
    {"add", (PyCFunction) Physics_add, METH_VARARGS, "add an object to the physics engine"},
    {"remove", (PyCFunction) Physics_remove, METH_VARARGS, "remove an object from the physics engine"},
    {"update", (PyCFunction) Physics_update, METH_NOARGS, "update the physics step"},
    {NULL}
};

PyTypeObject PhysicsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Physics",
    .tp_doc = "the physics engine",
    .tp_basicsize = sizeof(Physics),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Physics_new,
    .tp_init = (initproc) Physics_init,
    .tp_dealloc = (destructor) Physics_dealloc,
    .tp_as_sequence = &PhysicsSequenceMethods,
    .tp_getset = PhysicsGetSetters,
    .tp_methods = PhysicsMethods
};