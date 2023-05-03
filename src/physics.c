#include <main.h>

static void delete(Physics *self, PyObject *value) {
    if (BASE(value, BaseType)) {
        Base *base = (Base *) value;

        FOR(size_t, base -> length) {
            cpSpaceRemoveShape(self -> space, base -> shapes[i]);
            cpShapeFree(base -> shapes[i]);
        }

        cpSpaceRemoveBody(self -> space, base -> body);
        base -> length = 0;
        Py_DECREF(base);
    }

    else {
        Joint *joint = (Joint *) value;
        cpSpaceRemoveConstraint(self -> space, joint -> joint);
        Py_DECREF(joint);
    }
}

static int array(Physics *self, PyObject *check) {
    FOR(size_t, self -> length) {
        if (self -> data[i] != check) continue;
        delete(self, check);
        self -> length --;

        memmove(&self -> data[i], &self -> data[i + 1], sizeof NULL * (self -> length - i));
        return 0;
    }

    PyErr_SetString(PyExc_ValueError, "can't remove because it doesn't exist in physics engine");
    return -1;
}

static Py_ssize_t Physics_len(Physics *self) {
    return self -> length;
}

static PyObject *Physics_item(Physics *self, Py_ssize_t index) {
    if (index >= (signed) self -> length) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }

    return Py_INCREF(self -> data[index]), self -> data[index];
}

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

static double Physics_vecGravity(Physics *self, uint8_t index) {
    const cpVect vect = cpSpaceGetGravity(self -> space);
    const vec2 gravity = {vect.x, vect.y};
    return gravity[index];
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
    const cpVect gravity = cpSpaceGetGravity(self -> space);
    vec2 vect = {gravity.x, gravity.y};
    return vectorSet(value, vect, 2) ? -1 : cpSpaceSetGravity(self -> space, cpv(vect[x], vect[y])), 0;
}

static PyObject *Physics_add(Physics *self, PyObject *args) {
    Py_ssize_t length = PyTuple_GET_SIZE(args);
    self -> data = realloc(self -> data, sizeof NULL * (self -> length + length));

    FOR(Py_ssize_t, length) {
        PyObject *value = PyTuple_GET_ITEM(args, i);

        if (BASE(value, BaseType)) {
            Base *base = (Base *) value;

            if (base -> length) {
                PyErr_SetString(PyExc_ValueError, "already added to a physics engine");
                return NULL;
            }

            cpBodySetPosition(base -> body, cpv(base -> pos[x], base -> pos[y]));
            cpBodySetVelocity(base -> body, cpv(base -> vel[x], base -> vel[y]));
            cpSpaceAddBody(self -> space, base -> body);

            base -> new(base);
            baseMoment(base);

            FOR(size_t, base -> length) {
                cpShapeSetElasticity(base -> shapes[i], base -> elasticity);
                cpShapeSetFriction(base -> shapes[i], base -> friction);
                cpSpaceAddShape(self -> space, base -> shapes[i]);
            }
        }

        else if (BASE(value, JointType)) {
            Joint *joint = (Joint *) value;
            cpSpaceAddConstraint(self -> space, joint -> joint);
        }

        else {
            PyErr_SetString(PyExc_ValueError, "object must be Base or Joint");
            return NULL;
        }

        self -> data[self -> length] = value;
        self -> length ++;

        Py_INCREF(value);
    }

    Py_RETURN_NONE;
}

static PyObject *Physics_remove(Physics *self, PyObject *args) {
    FOR(Py_ssize_t, PyTuple_GET_SIZE(args))
        if (array(self, PyTuple_GET_ITEM(args, i)))
            return NULL;

    self -> data = realloc(self -> data, sizeof NULL * self -> length);
    Py_RETURN_NONE;
}

static PyObject *Physics_update(Physics *self, PyObject *Py_UNUSED(ignored)) {
    cpSpaceStep(self -> space, 1. / 60);

    FOR(size_t, self -> length) {
        if (BASE(self -> data[i], JointType)) continue;
        Base *base = (Base *) self -> data[i];

        cpVect pos = cpBodyGetPosition(base -> body);
        cpVect vel = cpBodyGetVelocity(base -> body);

        base -> pos[x] = pos.x;
        base -> pos[y] = pos.y;
        base -> vel[x] = vel.x;
        base -> vel[y] = vel.y;
    }
    
    Py_RETURN_NONE;
}

static PyObject *Physics_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Physics *self = (Physics *) type -> tp_alloc(type, 0);

    self -> data = malloc(0);
    self -> space = cpSpaceNew();

    return (PyObject *) self;
}

static int Physics_init(Physics *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"gravity_x", "gravity_y", NULL};
    cpVect vector = {0, -500};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dd", kwlist, &vector.x, &vector.y))
        return -1;

    FOR(size_t, self -> length) delete(self, self -> data[i]);
    self -> data = realloc(self -> data, 0);
    self -> length = 0;

    return cpSpaceSetGravity(self -> space, vector), 0;
}

static void Physics_dealloc(Physics *self) {
    FOR(size_t, self -> length) delete(self, self -> data[i]);
    cpSpaceFree(self -> space);
    free(self -> data);

    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PySequenceMethods PhysicsSequenceMethods = {
    .sq_length = (lenfunc) Physics_len,
    .sq_item = (ssizeargfunc) Physics_item
};

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
