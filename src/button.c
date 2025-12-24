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

static PyType_Slot button_slots[] = {
    {Py_tp_doc, "Represents the state of a keyboard or mouse button"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_str, button_str},
    {Py_tp_getset, button_getset},
    {Py_nb_bool, button_bool},
    {0}
};

Spec button_data = {{"Button", sizeof(Button), 0, Py_TPFLAGS_DEFAULT, button_slots}};