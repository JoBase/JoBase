#include <main.h>

static PyObject *Button_str(Button *self) {
    return PyUnicode_FromString(self -> key -> hold || self -> key -> release ? "True" : "False");
}

static PyObject *Button_get_press(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> press);
}

static PyObject *Button_get_release(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> release);
}

static PyObject *Button_get_repeat(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> repeat);
}

static PyObject *Button_get_hold(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> hold);
}

static int Button_bool(Button *self) {
    return self -> key -> hold || self -> key -> release;
}

Button *Button_new(Set *key) {
    Button *self = (Button *) PyObject_CallObject((PyObject *) &ButtonType, NULL);

    if (self) self -> key = key;
    return self;
}

static PyGetSetDef Button_getset[] = {
    {"press", (getter) Button_get_press, NULL, "the button is pressed", NULL},
    {"release", (getter) Button_get_release, NULL, "the button is released", NULL},
    {"repeat", (getter) Button_get_repeat, NULL, "the button repeat is triggered", NULL},
    {"hold", (getter) Button_get_hold, NULL, "the button is held down", NULL},
    {NULL}
};

static PyNumberMethods Button_as_number = {
    .nb_bool = (inquiry) Button_bool
};

PyTypeObject ButtonType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Button",
    .tp_doc = "represents the state of a keyboard or mouse button",
    .tp_basicsize = sizeof(Button),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_str = (reprfunc) Button_str,
    .tp_getset = Button_getset,
    .tp_as_number = &Button_as_number
};