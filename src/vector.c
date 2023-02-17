#define GET(e, i) e->get(e->parent, i)
#include <main.h>

enum {add, subtract, multiply, divide};

static PyObject *print(Vector *self, char a, char b) {
    char *buffer = malloc(14 * self -> size + 1);
    uint8_t count = 1;
    buffer[0] = a;

    FOR(uint8_t, self -> size) {
        if (i) {
            buffer[count ++] = ',';
            buffer[count ++] = ' ';
        }

        count += sprintf(&buffer[count], "%g", GET(self, i));
    }

    buffer[count] = b;
    PyObject *result = PyUnicode_FromString(buffer);

    return free(buffer), result;
}

static double check(double a, double b, uint8_t type) {
    switch (type) {
        case add: return a + b;
        case subtract: return a - b;
        case multiply: return a * b;
        case divide: return a / b;
    }

    return 0;
}

static PyObject *compare(double a, double b, int op) {
    if (op == Py_LT && a < b) Py_RETURN_TRUE;
    if (op == Py_GT && a > b) Py_RETURN_TRUE;
    if (op == Py_LE && a <= b) Py_RETURN_TRUE;
    if (op == Py_GE && a >= b) Py_RETURN_TRUE;
    
    Py_RETURN_FALSE;
}

static PyObject *number(Vector *self, PyObject *other, uint8_t type) {
    if (PyNumber_Check(other)) {
        PyObject *result = PyTuple_New(self -> size);

        const double value = PyFloat_AsDouble(other);
        if (ERR(value)) return NULL;

        FOR(uint8_t, self -> size) {
            PyObject *current = PyFloat_FromDouble(check(GET(self, i), value, type));
            if (!current) return NULL;

            PyTuple_SET_ITEM(result, i, current);
        }

        return result;
    }

    if (Py_TYPE(other) == &VectorType) {
        Vector *object = (Vector *) other;
        PyObject *result = PyTuple_New(MAX(self -> size, object -> size));

        FOR(uint8_t, MAX(self -> size, object -> size)) {
            PyObject *current = PyFloat_FromDouble(
                i >= self -> size ? GET(object, i) :
                i >= object -> size ? GET(self, i) :
                check(GET(self, i), GET(object, i), type));

            if (!current) return NULL;
            PyTuple_SET_ITEM(result, i, current);
        }

        return result;
    }

    format(PyExc_TypeError, "must be Vector or number, not %s", Py_TYPE(other) -> tp_name);
    return NULL;
}

static Py_ssize_t Vector_len(Vector *self) {
    return self -> size;
}

static PyObject *Vector_item(Vector *self, Py_ssize_t index) {
    if (index >= self -> size) return PyErr_SetString(PyExc_IndexError, "index out of range"), NULL;
    return PyFloat_FromDouble(GET(self, index));
}

static PyObject *Vector_add(Vector *self, PyObject *other) {
    return number(self, other, add);
}

static PyObject *Vector_subtract(Vector *self, PyObject *other) {
    return number(self, other, subtract);
}

static PyObject *Vector_multiply(Vector *self, PyObject *other) {
    return number(self, other, multiply);
}

static PyObject *Vector_trueDivide(Vector *self, PyObject *other) {
    return number(self, other, divide);
}

static void Vector_dealloc(Vector *self) {
    Py_DECREF(self -> parent);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyObject *Vector_richcompare(Vector *self, PyObject *other, int op) {
    if (PyNumber_Check(other)) {
        if (op == Py_EQ || op == Py_NE)
            Py_RETURN_FALSE;

        double a = 1;
        const double b = PyFloat_AsDouble(other);
        if (ERR(b)) return NULL;

        FOR(uint8_t, self -> size) a *= GET(self, i);
        compare(a, b, op);
    }

    if (Py_TYPE(other) == &VectorType) {
        Vector *object = (Vector *) other;

        if (op == Py_EQ || op == Py_NE) {
            bool ne = false;

            FOR(uint8_t, MIN(self -> size, object -> size))
                if (GET(self, i) != GET(object, i)) {
                    if (op == Py_EQ) Py_RETURN_FALSE;
                    ne = true;
                }

            if (op == Py_EQ || ne) Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }

        double a = 1, b = 1;
        FOR(uint8_t, self -> size) a *= GET(self, i);
        FOR(uint8_t, object -> size) b *= GET(object, i);
        compare(a, b, op);
    }

    format(PyExc_TypeError, "must be Vector or number, not %s", Py_TYPE(other) -> tp_name);
    return NULL;
}

static PyObject *Vector_str(Vector *self) {
    return print(self, '(', ')');
}

static PyObject *Vector_repr(Vector *self) {
    return print(self, '[', ']');
}

static PyObject *Vector_getattro(Vector *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return NULL;

    FOR(uint8_t, self -> size)
        if (!strcmp(name, self -> data[i].name))
            return PyFloat_FromDouble(GET(self, i));

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static int Vector_setattro(Vector *self, PyObject *attr, PyObject *value) {
    DEL(value)

    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return -1;
    
    FOR(uint8_t, self -> size)
        if (!strcmp(name, self -> data[i].name))
            return self -> data[i].set(self -> parent, value, NULL);

    return PyObject_GenericSetAttr((PyObject *) self, attr, value);
}

static PyNumberMethods VectorNumberMethods = {
    .nb_add = (binaryfunc) Vector_add,
    .nb_subtract = (binaryfunc) Vector_subtract,
    .nb_multiply = (binaryfunc) Vector_multiply,
    .nb_true_divide = (binaryfunc) Vector_trueDivide
};

static PySequenceMethods VectorSequenceMethods = {
    .sq_length = (lenfunc) Vector_len,
    .sq_item = (ssizeargfunc) Vector_item
};

Vector *vectorNew(PyObject *parent, Getter get, uint8_t size) {
    Vector *array = (Vector *) PyObject_CallObject((PyObject *) &VectorType, NULL);
    if (!array) return NULL;

    array -> parent = parent;
    array -> get = get;
    array -> size = size;

    return Py_INCREF(parent), array;
}

int vectorSet(PyObject *value, vec vector, uint8_t size) {
    DEL(value)

    if (Py_TYPE(value) == &VectorType) {
        Vector *object = (Vector *) value;

        FOR(uint8_t, MIN(size, object -> size))
            vector[i] = GET(object, i);
    }

    else if (PyNumber_Check(value)) {
        const double number = PyFloat_AsDouble(value);
        if (ERR(number)) return -1;

        FOR(uint8_t, size) vector[i] = number;
    }

    else if (PySequence_Check(value)) {
        PyObject *sequence = PySequence_Fast(value, NULL);
        Py_ssize_t length = PySequence_Fast_GET_SIZE(sequence);

        FOR(uint8_t, MIN(size, length)) {
            vector[i] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(sequence, i));

            if (ERR(vector[i])) {
                Py_DECREF(sequence);
                return -1;
            }
        }

        Py_DECREF(sequence);
    }

    else SEQ(value)
    return 0;
}


PyTypeObject VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Vector",
    .tp_doc = "represents a position, color or set of values",
    .tp_basicsize = sizeof(Vector),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Vector_dealloc,
    .tp_richcompare = (richcmpfunc) Vector_richcompare,
    .tp_str = (reprfunc) Vector_str,
    .tp_repr = (reprfunc) Vector_repr,
    .tp_getattro = (getattrofunc) Vector_getattro,
    .tp_setattro = (setattrofunc) Vector_setattro,
    .tp_as_number = &VectorNumberMethods,
    .tp_as_sequence = &VectorSequenceMethods
};