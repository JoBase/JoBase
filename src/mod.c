#include "main.h"

static PyObject *mod_getattro(PyObject *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);

    if (name) {
        Button *item = bsearch(name, keyboard.mod, keyboard.mods, sizeof(Button), (int (*)(const void *, const void *)) button_compare);
        return item ? Py_NewRef(item) : PyObject_GenericGetAttr(self, attr);
    }

    return NULL;
}

static PyType_Slot mod_slots[] = {
    {Py_tp_doc, "The input handler for keyboard modifiers"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_getattro, mod_getattro},
    {0}
};

Spec mod_data = {{"Mod", 0, 0, Py_TPFLAGS_DEFAULT, mod_slots}};