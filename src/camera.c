#include <main.h>

static PyObject *Camera_getX(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[x]);
}

static int Camera_setX(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> pos[x] = PyFloat_AsDouble(value);
    return ERR(self -> pos[x]) ? -1 : 0;
}

static PyObject *Camera_getY(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[y]);
}

static int Camera_setY(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> pos[y] = PyFloat_AsDouble(value);
    return ERR(self -> pos[y]) ? -1 : 0;
}

static vec Camera_vecPos(Camera *self) {
    return self -> pos;
}

static PyObject *Camera_getPos(Camera *self, void *Py_UNUSED(closure)) {
    Vector *pos = vectorNew((PyObject *) self, (Getter) Camera_vecPos, 2);

    pos -> data[x].set = (setter) Camera_setX;
    pos -> data[y].set = (setter) Camera_setY;
    pos -> data[x].name = "x";
    pos -> data[y].name = "y";

    return (PyObject *) pos;
}

static int Camera_setPos(Camera *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> pos, 2);
}

static PyObject *Camera_getTop(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[y] + windowSize()[y] / 2);
}

static PyObject *Camera_getBottom(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[y] - windowSize()[y] / 2);
}

static PyObject *Camera_getLeft(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[x] - windowSize()[x] / 2);
}

static PyObject *Camera_getRight(Camera *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> pos[x] + windowSize()[x] / 2);
}

static PyObject *Camera_moveToward(Camera *self, PyObject *args) {
    if (baseToward(self -> pos, args)) return NULL;
    Py_RETURN_NONE;
}

static PyObject *Camera_moveSmooth(Camera *self, PyObject *args) {
    if (baseSmooth(self -> pos, args)) return NULL;
    Py_RETURN_NONE;
}

static PyObject *Camera_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    camera = (Camera *) type -> tp_alloc(type, 0);

    Py_XINCREF(camera);
    return (PyObject *) camera;
}

static int Camera_init(Camera *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", NULL};

    self -> pos[x] = 0;
    self -> pos[y] = 0;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|sddO", kwlist, &self -> pos[x],
        &self -> pos[y]) ? 0 : -1;
}

static PyGetSetDef CameraGetSetters[] = {
    {"x", (getter) Camera_getX, (setter) Camera_setX, "x position of the camera", NULL},
    {"y", (getter) Camera_getY, (setter) Camera_setY, "y position of the camera", NULL},
    {"position", (getter) Camera_getPos, (setter) Camera_setPos, "position of the camera", NULL},
    {"pos", (getter) Camera_getPos, (setter) Camera_setPos, "position of the camera", NULL},
    {"top", (getter) Camera_getTop, NULL, "top position of the camera", NULL},
    {"bottom", (getter) Camera_getBottom, NULL, "bottom position of the camera", NULL},
    {"left", (getter) Camera_getLeft, NULL, "left position of the camera", NULL},
    {"right", (getter) Camera_getRight, NULL, "right position of the camera", NULL},
    {NULL}
};

static PyMethodDef CameraMethods[] = {
    {"move_toward", (PyCFunction) Camera_moveToward, METH_VARARGS, "move the camera toward another object"},
    {"move_smooth", (PyCFunction) Camera_moveSmooth, METH_VARARGS, "move the camera smoothly toward another object"},
    {NULL}
};

PyTypeObject CameraType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Camera",
    .tp_doc = "the user screen view and projection",
    .tp_basicsize = sizeof(Camera),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Camera_new,
    .tp_init = (initproc) Camera_init,
    .tp_getset = CameraGetSetters,
    .tp_methods = CameraMethods
};