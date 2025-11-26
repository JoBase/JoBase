#define FOR for(uint8_t i=0;i<self->size;i++)
#include "main.h"

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
    return self -> size == ((Vector *) value) -> size ?
        (Vector *) value : (PyErr_SetString(PyExc_BufferError, "Vector lengths must be the same"), NULL);
}

static PyObject *sequence(Vector *self, PyObject *value) {
    PyObject *array = PySequence_Fast(value, "Must be an iterable or a number");

    if (array && (self -> size != PySequence_Fast_GET_SIZE(array))) {
        PyErr_SetString(PyExc_BufferError, "Sequence must have the same length");
        return Py_DECREF(array), NULL;
    }

    return array;
}

static PyObject *tuple(Vector *self, PyObject *value, double method(double, double)) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) {
        if (Py_TYPE(value) == &VectorType) {
            Vector *other = vector(self, value);

            if (!other)
                return Py_DECREF(tuple), NULL;

            FOR if (enter(tuple, i, method(self -> var[i], other -> var[i])))
                return Py_DECREF(tuple), NULL;
        }

        else if (PyNumber_Check(value)) {
            const double number = PyFloat_AsDouble(value);

            if (ERR(number))
                return Py_DECREF(tuple), NULL;

            FOR if (enter(tuple, i, method(self -> var[i], number)))
                return Py_DECREF(tuple), NULL;
        }

        else {
            PyObject *list = sequence(self, value);

            if (!list)
                return Py_DECREF(tuple), NULL;

            FOR {
                const double value = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(list, i));

                if (ERR(value) || enter(tuple, i, method(self -> var[i], value))) {
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

        FOR self -> var[i] = method(self -> var[i], other -> var[i]);
    }

    else if (PyNumber_Check(value)) {
        const double number = PyFloat_AsDouble(value);
        if (ERR(number)) return NULL;

        FOR self -> var[i] = method(self -> var[i], number);
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

            self -> var[i] = method(self -> var[i], value);
        }

        Py_DECREF(list);
    }

    return self -> set && self -> set(self -> parent) ? NULL : (Py_INCREF(self), self);
}

static PyObject *print(Vector *self, char left, char right) {
    size_t total = 0;

    FOR total += snprintf(NULL, 0, "%g", self -> var[i]);
    char *buffer = malloc(total + (self -> size - 1) * 2 + 3);

    if (buffer) {
        char *ptr = buffer;

        *ptr ++ = left;
        FOR ptr += sprintf(ptr, i ? ", %g" : "%g", self -> var[i]);

        *ptr ++ = right;
        *ptr = 0;

        PyObject *res = PyUnicode_FromStringAndSize(buffer, ptr - buffer);
        return free(buffer), res;
    }

    return NULL;
}

static PyObject *vector_str(Vector *self) {
    return print(self, '(', ')');
}

static PyObject *vector_repr(Vector *self) {
    return print(self, '[', ']');
}

static PyObject *vector_getattro(Vector *self, PyObject *attr) {
    Py_ssize_t size;
    const char *name = PyUnicode_AsUTF8AndSize(attr, &size);

    if (name) {
        for (uint8_t i = 0; i < self -> size && size < 2; i ++)
            if (*name == self -> names[i])
                return PyFloat_FromDouble(self -> var[i]);

        return PyObject_GenericGetAttr((PyObject *) self, attr);
    }

    return NULL;
}

static int vector_setattro(Vector *self, PyObject *attr, PyObject *value) {
    Py_ssize_t size;

    const char *name = PyUnicode_AsUTF8AndSize(attr, &size);
    INIT(!name);

    for (uint8_t i = 0; i < self -> size && size < 2; i ++)
        if (*name == self -> names[i])
            return value ?
                ERR(self -> var[i] = PyFloat_AsDouble(value)) ? -1 : self -> set ? self -> set(self -> parent) :
                0 : (PyErr_Format(PyExc_AttributeError, "Cannot delete the '%c' attribute", *name), -1);

    return PyObject_GenericSetAttr((PyObject *) self, attr, value);
}

static Py_ssize_t vector_len(Vector *self) {
    return self -> size;
}

static PyObject *vector_item(Vector *self, Py_ssize_t index) {
    return index < self -> size ?
        PyFloat_FromDouble(self -> var[index]) : (PyErr_SetString(PyExc_IndexError, "Vector index out of range"), NULL);
}

static int vector_ass_item(Vector *self, Py_ssize_t index, PyObject *value) {
    return index < self -> size ?
        ERR(self -> var[index] = PyFloat_AsDouble(value)) ? -1 : self -> set ? self -> set(self -> parent) :
        0 : (PyErr_SetString(PyExc_IndexError, "Vector index out of range"), -1);
}

static PyObject *vector_add(Vector *self, PyObject *value) {
    return tuple(self, value, add);
}

static PyObject *vector_subtract(Vector *self, PyObject *value) {
    return tuple(self, value, subtract);
}

static PyObject *vector_multiply(Vector *self, PyObject *value) {
    return tuple(self, value, multiply);
}

static PyObject *vector_remainder(Vector *self, PyObject *value) {
    return tuple(self, value, fmod);
}

static PyObject *vector_floor_divide(Vector *self, PyObject *value) {
    return tuple(self, value, floor_divide);
}

static PyObject *vector_true_divide(Vector *self, PyObject *value) {
    return tuple(self, value, true_divide);
}

static Vector *vector_inplace_add(Vector *self, PyObject *value) {
    return inplace(self, value, add);
}

static Vector *vector_inplace_subtract(Vector *self, PyObject *value) {
    return inplace(self, value, subtract);
}

static Vector *vector_inplace_multiply(Vector *self, PyObject *value) {
    return inplace(self, value, multiply);
}

static Vector *vector_inplace_remainder(Vector *self, PyObject *value) {
    return inplace(self, value, fmod);
}

static Vector *vector_inplace_floor_divide(Vector *self, PyObject *value) {
    return inplace(self, value, floor_divide);
}

static Vector *vector_inplace_true_divide(Vector *self, PyObject *value) {
    return inplace(self, value, true_divide);
}

static PyObject *vector_negative(Vector *self) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) FOR if (enter(tuple, i, -self -> var[i]))
        return Py_DECREF(tuple), NULL;

    return tuple;
}

static PyObject *vector_positive(Vector *self) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) FOR if (enter(tuple, i, self -> var[i]))
        return Py_DECREF(tuple), NULL;

    return tuple;
}

static PyObject *vector_absolute(Vector *self) {
    PyObject *tuple = PyTuple_New(self -> size);

    if (tuple) FOR if (enter(tuple, i, fabs(self -> var[i])))
        return Py_DECREF(tuple), NULL;

    return tuple;
}

static int vector_bool(Vector *self) {
    FOR if (self -> var[i])
        return true;

    return false;
}

static PyObject *vector_get_length(Vector *self, void *closure) {
    double value = 0;

    FOR value += self -> var[i] * self -> var[i];
    return PyFloat_FromDouble(sqrt(value));
}

static void vector_dealloc(Vector *self) {
    Py_XDECREF(self -> parent);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

Vector *vector_new(PyObject *parent, double *vect, uint8_t size, int (*set)(PyObject *)) {
    Vector *self = PyObject_New(Vector, &VectorType);

    if (self) {
        Py_XINCREF(self -> parent = parent);

        self -> size = size;
        self -> var = vect;
        self -> set = set;
    }

    return self;
}

int vector_set(PyObject *value, double *vect, uint8_t size) {
    if (value) {
        if (Py_TYPE(value) == &VectorType)
            for (uint8_t i = 0; i < MIN(size, ((Vector *) value) -> size); i ++)
                vect[i] = ((Vector *) value) -> var[i];

        else if (PyNumber_Check(value)) {
            const double number = PyFloat_AsDouble(value);
            INIT(ERR(number))

            for (uint8_t i = 0; i < size; i ++)
                vect[i] = number;
        }

        else {
            PyObject *list = PySequence_Fast(value, "Must be an iterable or a number");
            INIT(!list)

            for (uint8_t i = 0; i < MIN(size, PySequence_Fast_GET_SIZE(list)); i ++)
                if (ERR(vect[i] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(list, i))))
                    return Py_DECREF(list), -1;

            Py_DECREF(list);
        }
    }

    return 0;
}

static PySequenceMethods vector_as_sequence = {
    .sq_length = (lenfunc) vector_len,
    .sq_item = (ssizeargfunc) vector_item,
    .sq_ass_item = (ssizeobjargproc) vector_ass_item
};

static PyNumberMethods vector_as_number = {
    .nb_add = (binaryfunc) vector_add,
    .nb_subtract = (binaryfunc) vector_subtract,
    .nb_multiply = (binaryfunc) vector_multiply,
    .nb_remainder = (binaryfunc) vector_remainder,
    .nb_floor_divide = (binaryfunc) vector_floor_divide,
    .nb_true_divide = (binaryfunc) vector_true_divide,
    .nb_inplace_add = (binaryfunc) vector_inplace_add,
    .nb_inplace_subtract = (binaryfunc) vector_inplace_subtract,
    .nb_inplace_multiply = (binaryfunc) vector_inplace_multiply,
    .nb_inplace_remainder = (binaryfunc) vector_inplace_remainder,
    .nb_inplace_floor_divide = (binaryfunc) vector_inplace_floor_divide,
    .nb_inplace_true_divide = (binaryfunc) vector_inplace_true_divide,
    .nb_negative = (unaryfunc) vector_negative,
    .nb_positive = (unaryfunc) vector_positive,
    .nb_absolute = (unaryfunc) vector_absolute,
    .nb_bool = (inquiry) vector_bool
};

static PyGetSetDef vector_getset[] = {
    {"length", (getter) vector_get_length, NULL, "Get the length of the vector", NULL},
    {NULL}
};

PyTypeObject VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Vector",
    .tp_doc = "Represents a position, color or set of values",
    .tp_basicsize = sizeof(Vector),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) vector_dealloc,
    .tp_str = (reprfunc) vector_str,
    .tp_repr = (reprfunc) vector_repr,
    .tp_getattro = (getattrofunc) vector_getattro,
    .tp_setattro = (setattrofunc) vector_setattro,
    .tp_as_sequence = &vector_as_sequence,
    .tp_as_number = &vector_as_number,
    .tp_getset = vector_getset
};