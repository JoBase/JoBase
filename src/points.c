#include "main.h"

static int update(Points *self) {
    return self -> update(self -> parent);
}

static PyObject *print(Points *self, char left, char right) {
    size_t total = 0;

    for (size_t i = 0; i < self -> parent -> len; i ++)
        total += snprintf(NULL, 0, "%g%g", self -> parent -> data[i].x, self -> parent -> data[i].y);

    char *buffer = malloc(total + self -> parent -> len * 6 + 1);

    if (buffer) {
        char *ptr = buffer;
        *ptr ++ = left;

        for (size_t i = 0; i < self -> parent -> len; i ++)
            ptr += sprintf(ptr, i ? ", %c%g, %g%c" : "%c%g, %g%c", left, self -> parent -> data[i].x, self -> parent -> data[i].y, right);

        *ptr ++ = right;
        *ptr = 0;

        PyObject *res = PyUnicode_FromStringAndSize(buffer, ptr - buffer);
        return free(buffer), res;
    }

    return PyErr_NoMemory();
}

static PyObject *points_str(Points *self) {
    return print(self, '(', ')');
}

static PyObject *points_repr(Points *self) {
    return print(self, '[', ']');
}

static Py_ssize_t points_len(Points *self) {
    return self -> parent -> len;
}

static Vector *points_item(Points *self, Py_ssize_t index) {
    if (index < (Py_ssize_t) self -> parent -> len) {
        Vector *vector = vector_new((PyObject *) self, (double *) &self -> parent -> data[index], 2, (int (*)(PyObject *)) update);

        if (vector) {
            vector -> names[x] = 'x';
            vector -> names[y] = 'y';
        }

        return vector;
    }

    return PyErr_SetString(PyExc_IndexError, "Points index out of range"), NULL;
}

static int points_ass_item(Points *self, Py_ssize_t index, PyObject *value) {
    return index < (Py_ssize_t) self -> parent -> len ?
        vector_set(value, (double *) &self -> parent -> data[index], 2) ? -1 :
        update(self) : (PyErr_SetString(PyExc_IndexError, "Points index out of range"), -1);
}

static void points_dealloc(Points *self) {
    Py_DECREF(self -> parent);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

Points *points_new(Shape *parent, int (*update)(Shape *)) {
    Points *self = PyObject_New(Points, points_data.type);

    if (self) {
        Py_INCREF(self -> parent = parent);
        self -> update = update;
    }

    return self;
}

int points_set(PyObject *value, Shape *shape) {
    if (value) {
        PyObject *list = PySequence_Fast(value, "Must be an iterable");
        INIT(!list)

        shape -> len = PySequence_Fast_GET_SIZE(list);
        shape -> data = realloc(shape -> data, shape -> len * sizeof(Vec2));

        if (!shape -> data) {
            Py_DECREF(list);
            return PyErr_NoMemory(), -1;
        }

        for (size_t i = 0; i < shape -> len; i ++)
            INIT(vector_set(PySequence_Fast_GET_ITEM(list, i), (double *) &shape -> data[i], 2));

        Py_DECREF(list);
    }

    return 0;
}

static PyType_Slot points_slots[] = {
    {Py_tp_doc, "Represents a list of coordinates"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_dealloc, points_dealloc},
    {Py_tp_str, points_str},
    {Py_tp_repr, points_repr},
    {Py_sq_length, points_len},
    {Py_sq_item, points_item},
    {Py_sq_ass_item, points_ass_item},
    {0}
};

Spec points_data = {{"Points", sizeof(Points), 0, Py_TPFLAGS_DEFAULT, points_slots}};