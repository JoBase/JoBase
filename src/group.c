#include <main.h>

Group *Group_new(int id) {
    Group *self = (Group *) PyObject_CallObject((PyObject *) &GroupType, NULL);
    if (self) self -> id = id;

    return self;
}

PyTypeObject GroupType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Group",
    .tp_doc = "prevent collision between a group of bodies",
    .tp_basicsize = sizeof(Group),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew
};