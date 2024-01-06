#include <main.h>

static double add(double a, double b) {
    return a + b;
}

static double subtract(double a, double b) {
    return a - b;
}

static double multiply(double a, double b) {
    return a * b;
}

static double floor_divide(double a, double b) {
    return floor(a / b);
}

static double true_divide(double a, double b) {
    return a / b;
}

static int enter(PyObject *tuple, Py_ssize_t index, double value) {
    PyObject *result = PyFloat_FromDouble(value);
    return result ? PyTuple_SET_ITEM(tuple, index, result), 0 : -1;
}

static Vector *vector(Vector *self, PyObject *value) {
    #define FOR for(uint8_t i=0;i<self->size;i++)
    #define SET self->set&&self->set(self->parent)

    if (self -> size != ((Vector *) value) -> size) {
        PyErr_SetString(PyExc_BufferError, "vector lengths must be the same");
        return NULL;
    }

    return (Vector *) value;
}

static PyObject *sequence(Vector *self, PyObject *value) {
    PyObject *array = PySequence_Fast(value, "must be an iterable or a number");

    if (array) {
        if (self -> size != PySequence_Fast_GET_SIZE(array)) {
            PyErr_SetString(PyExc_BufferError, "sequence must have the same length");
            Py_DECREF(array);
            return NULL;
        }

        return array;
    }

    return NULL;
}

static PyObject *tuple(Vector *self, PyObject *value, double method(double, double)) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) {
        if (Py_TYPE(value) == &VectorType) {
            Vector *other = vector(self, value);

            if (!other) {
                Py_DECREF(tuple);
                return NULL;
            }

            FOR if (enter(tuple, i, method(self -> vect[i], other -> vect[i]))) {
                Py_DECREF(tuple);
                return NULL;
            }
        }

        else if (PyNumber_Check(value)) {
            const double number = PyFloat_AsDouble(value);

            if (ERR(number)) {
                Py_DECREF(tuple);
                return NULL;
            }

            FOR if (enter(tuple, i, method(self -> vect[i], number))) {
                Py_DECREF(tuple);
                return NULL;
            }
        }

        else {
            PyObject *list = sequence(self, value);

            if (!list) {
                Py_DECREF(tuple);
                return NULL;
            }

            FOR {
                const double value = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(list, i));

                if (ERR(value) || enter(tuple, i, method(self -> vect[i], value))) {
                    Py_DECREF(list);
                    Py_DECREF(tuple);
                    return NULL;
                }
            }

            Py_DECREF(list);
        }
    }

    return tuple;
}

static Vector *inplace(Vector *self, PyObject *value, double method(double, double)) {
    if (Py_TYPE(value) == &VectorType) {
        Vector *other = vector(self, value);
        if (!other) return NULL;

        FOR self -> vect[i] = method(self -> vect[i], other -> vect[i]);
    }

    else if (PyNumber_Check(value)) {
        const double number = PyFloat_AsDouble(value);
        if (ERR(number)) return NULL;

        FOR self -> vect[i] = method(self -> vect[i], number);
    }

    else {
        PyObject *list = sequence(self, value);
        if (!list) return NULL;

        FOR {
            const double value = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(list, i));

            if (ERR(value)) {
                Py_DECREF(list);
                return NULL;
            }

            self -> vect[i] = method(self -> vect[i], value);
        }

        Py_DECREF(list);
    }

    return SET ? NULL : (Py_INCREF(self), self);
}

static PyObject *print(Vector *self, char *brackets) {
    uint8_t size = 2;
    char *buffer = malloc(size);

    FOR {
        char *format = i ? ", %g" : "%g";
        const int length = snprintf(NULL, 0, format, self -> vect[i]);
        buffer = realloc(buffer, size + length);

        sprintf(&buffer[size - 1], format, self -> vect[i]);
        size += length;
    }

    *buffer = brackets[x];
    buffer[size - 1] = brackets[y];

    PyObject *result = PyUnicode_FromStringAndSize(buffer, size);
    return free(buffer), result;
}

static PyObject *Vector_str(Vector *self) {
    return print(self, "()");
}

static PyObject *Vector_repr(Vector *self) {
    return print(self, "[]");
}

static PyObject *Vector_getattro(Vector *self, PyObject *attr) {
    Py_ssize_t size;
    const char *name = PyUnicode_AsUTF8AndSize(attr, &size);

    if (name) {
        for (uint8_t i = 0; i < self -> size && size < 2; i ++)
            if (*name == self -> names[i])
                return PyFloat_FromDouble(self -> vect[i]);

        return PyObject_GenericGetAttr((PyObject *) self, attr);
    }

    return NULL;
}

static int Vector_setattro(Vector *self, PyObject *attr, PyObject *value) {
    Py_ssize_t size;
    const char *name = PyUnicode_AsUTF8AndSize(attr, &size);

    if (name) {
        for (uint8_t i = 0; i < self -> size && size < 2; i ++)
            if (*name == self -> names[i]) {
                if (!value) {
                    PyErr_Format(PyExc_AttributeError, "cannot delete the %c attribute", *name);
                    return -1;
                }

                return ERR(self -> vect[i] = PyFloat_AsDouble(value)) ? -1 : SET;
            }

        return PyObject_GenericSetAttr((PyObject *) self, attr, value);
    }

    return -1;
}

static Py_ssize_t Vector_len(Vector *self) {
    return self -> size;
}

static PyObject *Vector_item(Vector *self, Py_ssize_t index) {
    if (index < self -> size)
        return PyFloat_FromDouble(self -> vect[index]);

    return PyErr_SetString(PyExc_IndexError, "vector index out of range"), NULL;
}

static PyObject *Vector_add(Vector *self, PyObject *value) {
    return tuple(self, value, add);
}

static PyObject *Vector_subtract(Vector *self, PyObject *value) {
    return tuple(self, value, subtract);
}

static PyObject *Vector_multiply(Vector *self, PyObject *value) {
    return tuple(self, value, multiply);
}

static PyObject *Vector_remainder(Vector *self, PyObject *value) {
    return tuple(self, value, fmod);
}

static PyObject *Vector_floor_divide(Vector *self, PyObject *value) {
    return tuple(self, value, floor_divide);
}

static PyObject *Vector_true_divide(Vector *self, PyObject *value) {
    return tuple(self, value, true_divide);
}

static Vector *Vector_inplace_add(Vector *self, PyObject *value) {
    return inplace(self, value, add);
}

static Vector *Vector_inplace_subtract(Vector *self, PyObject *value) {
    return inplace(self, value, subtract);
}

static Vector *Vector_inplace_multiply(Vector *self, PyObject *value) {
    return inplace(self, value, multiply);
}

static Vector *Vector_inplace_remainder(Vector *self, PyObject *value) {
    return inplace(self, value, fmod);
}

static Vector *Vector_inplace_floor_divide(Vector *self, PyObject *value) {
    return inplace(self, value, floor_divide);
}

static Vector *Vector_inplace_true_divide(Vector *self, PyObject *value) {
    return inplace(self, value, true_divide);
}

static PyObject *Vector_negative(Vector *self) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) FOR if (enter(tuple, i, -self -> vect[i])) {
        Py_DECREF(tuple);
        return NULL;
    }

    return tuple;
}

static PyObject *Vector_positive(Vector *self) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) FOR if (enter(tuple, i, self -> vect[i])) {
        Py_DECREF(tuple);
        return NULL;
    }

    return tuple;
}

static PyObject *Vector_absolute(Vector *self) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) FOR if (enter(tuple, i, fabs(self -> vect[i]))) {
        Py_DECREF(tuple);
        return NULL;
    }

    return tuple;
}

static PyObject *Vector_bool(Vector *self) {
    FOR if (self -> vect[i])
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *Vector_get_length(Vector *self, void *closure) {
    double value = 0;

    FOR value += self -> vect[i] * self -> vect[i];
    return PyFloat_FromDouble(sqrt(value));
}

static void Vector_dealloc(Vector *self) {
    Py_DECREF(self -> parent);
    VectorType.tp_free(self);
}

Vector *Vector_new(PyObject *parent, vec vect, uint8_t size, set set) {
    Vector *self = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);

    if (self) {
        Py_INCREF(parent);

        self -> parent = parent;
        self -> size = size;
        self -> vect = vect;
        self -> set = set;
    }

    return self;
}

int Vector_set(PyObject *value, vec vect, uint8_t size) {
    if (value) {
        if (Py_TYPE(value) == &VectorType) {
            Vector *other = (Vector *) value;

            for (uint8_t i = 0; i < MIN(size, other -> size); i ++)
                vect[i] = other -> vect[i];
        }

        else if (PyNumber_Check(value)) {
            const double number = PyFloat_AsDouble(value);
            INIT(ERR(number))

            for (uint8_t i = 0; i < size; i ++)
                vect[i] = number;
        }

        else {
            PyObject *list = PySequence_Fast(value, "must be an iterable or a number");
            INIT(!list)

            for (uint8_t i = 0; i < MIN(size, PySequence_Fast_GET_SIZE(list)); i ++) {
                vect[i] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(list, i));

                if (ERR(vect[i])) {
                    Py_DECREF(list);
                    return -1;
                }
            }

            Py_DECREF(list);
        }
    }

    return 0;
}

static PySequenceMethods Vector_as_sequence = {
    .sq_length = (lenfunc) Vector_len,
    .sq_item = (ssizeargfunc) Vector_item
};

static PyNumberMethods Vector_as_number = {
    .nb_add = (binaryfunc) Vector_add,
    .nb_subtract = (binaryfunc) Vector_subtract,
    .nb_multiply = (binaryfunc) Vector_multiply,
    .nb_remainder = (binaryfunc) Vector_remainder,
    .nb_floor_divide = (binaryfunc) Vector_floor_divide,
    .nb_true_divide = (binaryfunc) Vector_true_divide,
    .nb_inplace_add = (binaryfunc) Vector_inplace_add,
    .nb_inplace_subtract = (binaryfunc) Vector_inplace_subtract,
    .nb_inplace_multiply = (binaryfunc) Vector_inplace_multiply,
    .nb_inplace_remainder = (binaryfunc) Vector_inplace_remainder,
    .nb_inplace_floor_divide = (binaryfunc) Vector_inplace_floor_divide,
    .nb_inplace_true_divide = (binaryfunc) Vector_inplace_true_divide,
    .nb_negative = (unaryfunc) Vector_negative,
    .nb_positive = (unaryfunc) Vector_positive,
    .nb_absolute = (unaryfunc) Vector_absolute,
    .nb_bool = (inquiry) Vector_bool
};

static PyGetSetDef Vector_getset[] = {
    {"length", (getter) Vector_get_length, NULL, "get the length of the vector", NULL},
    {NULL}
};

PyTypeObject VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Vector",
    .tp_doc = "represents a position, color or set of values",
    .tp_basicsize = sizeof(Vector),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Vector_dealloc,
    .tp_str = (reprfunc) Vector_str,
    .tp_repr = (reprfunc) Vector_repr,
    .tp_getattro = (getattrofunc) Vector_getattro,
    .tp_setattro = (setattrofunc) Vector_setattro,
    .tp_as_sequence = &Vector_as_sequence,
    .tp_as_number = &Vector_as_number,
    .tp_getset = Vector_getset,
};