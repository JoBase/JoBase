#include <main.h>

static int gravity(Physics *self) {
    return cpSpaceSetGravity(self -> space, cpv(self -> gravity.x, self -> gravity.y)), 0;
}

static Vector *Physics_get_gravity(Physics *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> gravity, 2, (set) gravity);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Physics_set_gravity(Physics *self, PyObject *value, void *closure) {
    DEL(value, "gravity")
    return Vector_set(value, (vec) &self -> gravity, 2) ? -1 : gravity(self);
}

static Body *Physics_body(Physics *self, PyObject *args) {
    return Body_new(self, args);
}

static Group *Physics_group(Physics *self, PyObject *args) {
    return self -> group ++, Group_new(self -> group);
}

static PyObject *Physics_update(Physics *self, PyObject *args) {
    cpSpaceStep(self -> space, 1. / 60);

    for (Body *this = self -> list; this; this = this -> next) {
        cpFloat angle = cpBodyGetAngle(this -> body);
        cpVect velocity = cpBodyGetVelocity(this -> body);
        cpVect pos = cpBodyGetPosition(this -> body);
        cpVect rot = cpvforangle(angle);

        this -> velocity.x = velocity.x;
        this -> velocity.y = velocity.y;

        for (Base *base = this -> list; base; base = base -> next) {
            cpVect value = cpvadd(cpvrotate(cpv(base -> transform.x, base -> transform.y), rot), pos);

            base -> pos.x = value.x;
            base -> pos.y = value.y;
            base -> angle = base -> rotate + angle * 180 / M_PI;
        }
    }

    Py_RETURN_NONE;
}

static Physics *Physics_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Physics *self = (Physics *) type -> tp_alloc(type, 0);

    if (self) {
        self -> space = cpSpaceNew();
        self -> group = 0;
    }

    return self;
}

static int Physics_init(Physics *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"gravity_x", "gravity_y", NULL};

    self -> gravity.x = 0;
    self -> gravity.y = -500;

    INIT(!PyArg_ParseTupleAndKeywords(args, kwds, "|dd:Physics", kwlist, &self -> gravity.x, &self -> gravity.y))
    return gravity(self);
}

static void Physics_dealloc(Physics *self) {
    cpSpaceFree(self -> space);
    PhysicsType.tp_free(self);
}

static PyGetSetDef Physics_getset[] = {
    {"gravity", (getter) Physics_get_gravity, (setter) Physics_set_gravity, "the gravity of the physics engine world", NULL},
    {NULL}
};

static PyMethodDef Physics_methods[] = {
    {"body", (PyCFunction) Physics_body, METH_VARARGS, "create a new body for this physics engine"},
    {"group", (PyCFunction) Physics_group, METH_NOARGS, "create a new collision group for this physics engine"},
    {"update", (PyCFunction) Physics_update, METH_NOARGS, "update the physics step"},
    {NULL}
};

PyTypeObject PhysicsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Physics",
    .tp_doc = "a super fast rigid body physics engine",
    .tp_basicsize = sizeof(Physics),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc) Physics_new,
    .tp_init = (initproc) Physics_init,
    .tp_dealloc = (destructor) Physics_dealloc,
    .tp_methods = Physics_methods,
    .tp_getset = Physics_getset
};