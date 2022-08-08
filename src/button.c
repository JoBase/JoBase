#include <main.h>

static PyObject *Button_str(Button *self) {
    return PyUnicode_FromString(self -> state -> hold || self -> state -> release ? "True" : "False");
}

static PyObject *Button_getPress(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> press);
}

static PyObject *Button_getRelease(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> release);
}

static PyObject *Button_getRepeat(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> repeat);
}

static PyObject *Button_getHold(Button *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> state -> hold);
}

static int Button_bool(Button *self) {
    return self -> state -> hold || self -> state -> release;
}

static PyGetSetDef ButtonGetSetters[] = {
    {"press", (getter) Button_getPress, NULL, "the button is pressed", NULL},
    {"release", (getter) Button_getRelease, NULL, "the button is released", NULL},
    {"repeat", (getter) Button_getRepeat, NULL, "the button repeat is triggered", NULL},
    {"hold", (getter) Button_getHold, NULL, "the button is held down", NULL},
    {NULL}
};

static PyNumberMethods ButtonNumberMethods = {
    .nb_bool = (inquiry) Button_bool
};

Button *buttonNew(Set *state) {
    Button *button = (Button *) PyObject_CallObject((PyObject *) &ButtonType, NULL);
    return button ? button -> state = state, button : NULL;
}

PyTypeObject ButtonType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Button",
    .tp_doc = "represents the state of a keyboard or mouse button",
    .tp_basicsize = sizeof(Button),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_str = (reprfunc) Button_str,
    .tp_getset = ButtonGetSetters,
    .tp_as_number = &ButtonNumberMethods
};