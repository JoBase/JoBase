#include <main.h>

static PyObject *Camera_get_x(Camera *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.x);
}

static int Camera_set_x(Camera *self, PyObject *value, void *closure) {
    DEL(value, "x")
    return ERR(self -> pos.x = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *Camera_get_y(Camera *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.y);
}

static int Camera_set_y(Camera *self, PyObject *value, void *closure) {
    DEL(value, "y")
    return ERR(self -> pos.y = PyFloat_AsDouble(value)) ? -1 : 0;
}

static Vector *Camera_get_pos(Camera *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> pos, 2, NULL);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Camera_set_pos(Camera *self, PyObject *value, void *closure) {
    DEL(value, "pos")
    return Vector_set(value, (vec) &self -> pos, 2);
}

static PyObject *Camera_get_top(Camera *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.y + window -> size.y / 2);
}

static int Camera_set_top(Camera *self, PyObject *value, void *closure) {
    DEL(value, "top")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.y = result - window -> size.y / 2, 0);
}

static PyObject *Camera_get_bottom(Camera *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.y - window -> size.y / 2);
}

static int Camera_set_bottom(Camera *self, PyObject *value, void *closure) {
    DEL(value, "bottom")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.y = result + window -> size.y / 2, 0);
}

static PyObject *Camera_get_left(Camera *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.x - window -> size.x / 2);
}

static int Camera_set_left(Camera *self, PyObject *value, void *closure) {
    DEL(value, "left")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.x = result + window -> size.x / 2, 0);
}

static PyObject *Camera_get_right(Camera *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.x + window -> size.x / 2);
}

static int Camera_set_right(Camera *self, PyObject *value, void *closure) {
    DEL(value, "right")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.x = result - window -> size.x / 2, 0);
}

static int Camera_init(Camera *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", NULL};

    self -> pos.x = 0;
    self -> pos.y = 0;

    self -> scale.x = 1;
    self -> scale.y = 1;

    return PyArg_ParseTupleAndKeywords(args, kwds, "|dd:Camera", kwlist, &self -> pos.x, &self -> pos.y) ? 0 : -1;
}

static PyGetSetDef Camera_getset[] = {
    {"x", (getter) Camera_get_x, (setter) Camera_set_x, "x position of the camera", NULL},
    {"y", (getter) Camera_get_y, (setter) Camera_set_y, "y position of the camera", NULL},
    {"position", (getter) Camera_get_pos, (setter) Camera_set_pos, "position of the camera", NULL},
    {"pos", (getter) Camera_get_pos, (setter) Camera_set_pos, "position of the camera", NULL},
    {"top", (getter) Camera_get_top, (setter) Camera_set_top, "top position of the camera", NULL},
    {"bottom", (getter) Camera_get_bottom, (setter) Camera_set_bottom, "bottom position of the camera", NULL},
    {"left", (getter) Camera_get_left, (setter) Camera_set_left, "left position of the camera", NULL},
    {"right", (getter) Camera_get_right, (setter) Camera_set_right, "right position of the camera", NULL},
    {NULL}
};

PyTypeObject CameraType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Camera",
    .tp_doc = "represents the user's view of the game",
    .tp_basicsize = sizeof(Camera),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Camera_init,
    // .tp_methods = Camera_methods,
    .tp_getset = Camera_getset
};