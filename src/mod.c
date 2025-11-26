#include "main.h"

static PyObject *mod_getattro(PyObject *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);

    if (name) {
        Button *item = bsearch(name, keyboard.mod, keyboard.mods, sizeof(Button), (int (*)(const void *, const void *)) button_compare);
        return item ? Py_NewRef(item) : PyObject_GenericGetAttr(self, attr);
    }

    return NULL;
}

PyTypeObject ModType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Mod",
    .tp_doc = "Input handler for keyboard modifiers",
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_getattro = mod_getattro
};