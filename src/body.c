#include <main.h>

static int check(Body *self, cpBodyType type) {
    if (type != CP_BODY_TYPE_DYNAMIC && type != CP_BODY_TYPE_KINEMATIC) {
        PyErr_SetString(PyExc_ValueError, "type must be DYNAMIC or STATIC");
        return -1;
    }

    return cpBodySetType(self -> body, type), 0;
}

static int velocity(Body *self) {
    return cpBodySetVelocity(self -> body, cpv(self -> velocity.x, self -> velocity.y)), 0;
}

static PyObject *Body_get_type(Body *self, void *closure) {
    return PyLong_FromLong(cpBodyGetType(self -> body));
}

static int Body_set_type(Body *self, PyObject *value, void *closure) {
    DEL(value, "type")

    const long type = PyLong_AsLong(value);
    if (ERR(type)) return -1;

    return check(self, type);
}

static Vector *Body_get_velocity(Body *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> velocity, 2, (set) velocity);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Body_set_velocity(Body *self, PyObject *value, void *closure) {
    DEL(value, "velocity")
    return Vector_set(value, (vec) &self -> velocity, 2) ? -1 : velocity(self);
}

static void Body_dealloc(Body *self) {
    cpSpaceRemoveBody(self -> parent -> space, self -> body);
    cpBodyFree(self -> body);

    REM(Body, self -> parent -> list, self)
    Py_CLEAR(self -> parent);

    BodyType.tp_free(self);
}

Body *Body_new(Physics *parent, PyObject *args) {
    Body *self = (Body *) PyObject_CallObject((PyObject *) &BodyType, NULL);

    if (self) {
        double type = CP_BODY_TYPE_DYNAMIC;

        self -> next = parent -> list;
        self -> body = cpSpaceAddBody(parent -> space, cpBodyNew(0, 0));

        if (!PyArg_ParseTuple(args, "|d:Body", &type) || check(self, type))
            return NULL;

        parent -> list = self;
        Py_INCREF(self -> parent = parent);
    }

    return self;
}

cpVect Body_set(Body *body, Vec2 pos) {
    return cpvrotate(cpvsub(cpv(pos.x, pos.y), cpBodyGetPosition(body -> body)), cpvforangle(cpBodyGetAngle(body -> body)));
}

Vec2 Body_get(Body *body, cpVect pos) {
    cpVect result = cpvadd(cpvrotate(pos, cpvforangle(cpBodyGetAngle(body -> body))), cpBodyGetPosition(body -> body));
    Vec2 anchor = {result.x, result.y};

    return anchor;
}

static PyGetSetDef Body_getset[] = {
    {"type", (getter) Body_get_type, (setter) Body_set_type, "this can be STATIC or DYNAMIC, determining how the body moves", NULL},
    {"velocity", (getter) Body_get_velocity, (setter) Body_set_velocity, "the speed of the body", NULL},
    {"speed", (getter) Body_get_velocity, (setter) Body_set_velocity, "the speed of the body", NULL},
    {NULL}
};

PyTypeObject BodyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Body",
    .tp_doc = "create a rigid body for the physics engine",
    .tp_basicsize = sizeof(Body),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Body_dealloc,
    .tp_getset = Body_getset
};