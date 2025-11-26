#include "main.h"

static PyObject *key_get_mod(PyObject *self, void *closure) {
    return Py_NewRef(&keyboard.map);
}

static PyObject *key_get_press(PyObject *self, void *closure) {
    return PyBool_FromLong(keyboard.press);
}

static PyObject *key_get_release(PyObject *self, void *closure) {
    return PyBool_FromLong(keyboard.release);
}

static PyObject *key_getattro(PyObject *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);

    if (name) {
        Button *item = bsearch(name, keyboard.key, keyboard.keys, sizeof(Button), (int (*)(const void *, const void *)) button_compare);
        return item ? Py_NewRef(item) : PyObject_GenericGetAttr(self, attr);
    }

    return NULL;
}

static PyGetSetDef key_getset[] = {
    {"mod", (getter) key_get_mod, NULL, "The input handler for keyboard modifiers", NULL},
    {"press", key_get_press, NULL, "A key is pressed", NULL},
    {"release", key_get_release, NULL, "A key is released", NULL},
    {NULL}
};

PyTypeObject KeyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Key",
    .tp_doc = "The input handler for the keyboard",
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_getattro = key_getattro,
    .tp_getset = key_getset
};