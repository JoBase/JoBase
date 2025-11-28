#include "main.h"

static int sound_init(Rect *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", NULL};

    const char *name = NULL;

    return PyArg_ParseTupleAndKeywords(args, kwds, "|s:Sound", kwlist, &name) ? 0 : -1;
}

PyTypeObject SoundType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Sound",
    .tp_doc = "Play audio and sound effects",
    .tp_basicsize = sizeof(Sound),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) sound_init,
    // .tp_getset = rect_getset,
    // .tp_methods = rect_methods
};