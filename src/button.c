#include "main.h"

static PyObject *button_str(Button *self) {
    return PyUnicode_FromString(self -> key -> down || self -> key -> press ? "True" : "False");
}

static PyObject *button_get_press(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> press);
}

static PyObject *button_get_release(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> release);
}

static PyObject *button_get_repeat(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> repeat);
}

static PyObject *button_get_down(Button *self, void *closure) {
    return PyBool_FromLong(self -> key -> down);
}

static int button_bool(Button *self) {
    return self -> key -> down || self -> key -> press;
}

int button_compare(const char *code, Button *button) {
    return strcmp(code, button -> key -> key);
}

static PyGetSetDef button_getset[] = {
    {"press", (getter) button_get_press, NULL, "The button is pressed down", NULL},
    {"release", (getter) button_get_release, NULL, "The button is released", NULL},
    {"repeat", (getter) button_get_repeat, NULL, "The button repeat is triggered", NULL},
    {"down", (getter) button_get_down, NULL, "The button is held down", NULL},
    {NULL}
};

static PyNumberMethods button_as_number = {
    .nb_bool = (inquiry) button_bool
};

PyTypeObject ButtonType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Button",
    .tp_doc = "Represents the state of a keyboard or mouse button",
    .tp_basicsize = sizeof(Button),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_str = (reprfunc) button_str,
    .tp_getset = button_getset,
    .tp_as_number = &button_as_number
};