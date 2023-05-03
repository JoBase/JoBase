#include <main.h>

static PyObject *Motor_getSpeed(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cpSimpleMotorGetRate(self -> joint));
}

static int Motor_setSpeed(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double speed = PyFloat_AsDouble(value);
    return ERR(speed) ? -1 : cpSimpleMotorSetRate(self -> joint, speed), 0;
}

static PyObject *Motor_draw(Joint *self, PyObject *Py_UNUSED(ignored)) {
    vec a = self -> a -> pos;
    vec b = self -> b -> pos;

    vec2 base[] = {{a[x], a[y]}, {b[x], b[y]}};
    jointDraw(self, base, 2);

    Py_RETURN_NONE;
}

static PyObject *Motor_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return jointNew(type, (cpConstraint *) cpSimpleMotorAlloc());
}

static int Motor_init(Joint *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"a", "b", "speed", "width", "color", NULL};

    jointInit(self);
    PyObject *color = NULL;
    double speed = 0;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "O!O!|ddO", kwlist, &BaseType, &self -> a, &BaseType,
        &self -> b, &speed, &self -> width, &color)) return -1;

    cpSimpleMotorInit((cpSimpleMotor *) self -> joint, self -> a -> body, self -> b -> body, speed);
    return jointStart(self, color);
}

static PyGetSetDef MotorGetSetters[] = {
    {"speed", (getter) Motor_getSpeed, (setter) Motor_setSpeed, "speed of the motor", NULL},
    {NULL}
};

static PyMethodDef MotorMethods[] = {
    {"draw", (PyCFunction) Motor_draw, METH_NOARGS, "draw the motor joint on the screen"},
    {NULL}
};

PyTypeObject MotorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Motor",
    .tp_doc = "applies a rotational force between two bodies",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &JointType,
    .tp_new = Motor_new,
    .tp_init = (initproc) Motor_init,
    .tp_getset = MotorGetSetters,
    .tp_methods = MotorMethods
};