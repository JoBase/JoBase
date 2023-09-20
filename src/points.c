#include <main.h>

static int update(Points *self) {
    return self -> method(self -> parent);
}

static Py_ssize_t Points_len(Points *self) {
    return self -> parent -> length;
}

static Vector *Points_item(Points *self, Py_ssize_t index) {
    if (index < (Py_ssize_t) self -> parent -> length) {
        Vector *vector = Vector_new((PyObject *) self, (vec) &self -> parent -> points[index], 2, (set) update);

        if (vector) {
            vector -> names[x] = 'x';
            vector -> names[y] = 'y';
        }

        return vector;
    }

    return PyErr_SetString(PyExc_IndexError, "index out of range"), NULL;
}

static int Points_ass_item(Points *self, Py_ssize_t index, PyObject *value) {
    if (index < (Py_ssize_t) self -> parent -> length)
        return Vector_set(value, (vec) &self -> parent -> points[index], 2) ? -1 : update(self);

    return PyErr_SetString(PyExc_IndexError, "index out of range"), -1;
}

static void Points_dealloc(Points *self) {
    Py_DECREF(self -> parent);
    PointsType.tp_free(self);
}

Points *Points_new(Shape *parent, reset method) {
    Points *self = (Points *) PyObject_CallObject((PyObject *) &PointsType, NULL);

    if (self) {
        Py_INCREF(parent);

        self -> parent = parent;
        self -> method = method;
    }

    return self;
}

int Points_set(Shape *self, PyObject *value) {
    if (value) {
        PyObject *list = PySequence_Fast(value, "must be an iterable");
        INIT(!list)

        self -> length = PySequence_Fast_GET_SIZE(list);
        self -> points = realloc(self -> points, self -> length * sizeof(Vec2));

        for (size_t i = 0; i < self -> length; i ++) {
            PyObject *point = PySequence_Fast_GET_ITEM(list, i);
            PyObject *array = PySequence_Fast(point, "values must be iterable");

            if (!array) {
                Py_DECREF(list);
                return -1;
            }

            if (PySequence_Fast_GET_SIZE(array) < 2) {
                PyErr_SetString(PyExc_ValueError, "point must contain 2 values");

                Py_DECREF(array);
                Py_DECREF(list);
                return -1;
            }

            self -> points[i].x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(array, x));
            self -> points[i].y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(array, y));
            Py_DECREF(array);

            if (ERR(self -> points[i].x) || ERR(self -> points[i].y)) {
                Py_DECREF(list);
                return -1;
            }
        }

        Py_DECREF(list);
    }

    return 0;
}

static PySequenceMethods Points_as_sequence = {
    .sq_length = (lenfunc) Points_len,
    .sq_item = (ssizeargfunc) Points_item,
    .sq_ass_item = (ssizeobjargproc) Points_ass_item
};

PyTypeObject PointsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Points",
    .tp_doc = "a set of coordinates that make a polygon",
    .tp_basicsize = sizeof(Points),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Points_dealloc,
    .tp_as_sequence = &Points_as_sequence
};