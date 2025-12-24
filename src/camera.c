#include "main.h"

static PyObject *camera_get_x(PyObject *self, void *closure) {
    return PyFloat_FromDouble(camera.pos.x);
}

static int camera_set_x(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "x")
    return ERR(camera.pos.x = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *camera_get_y(PyObject *self, void *closure) {
    return PyFloat_FromDouble(camera.pos.y);
}

static int camera_set_y(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "y")
    return ERR(camera.pos.y = PyFloat_AsDouble(value)) ? -1 : 0;
}

static Vector *camera_get_pos(PyObject *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &camera.pos, 2, NULL);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int camera_set_pos(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "pos")
    return vector_set(value, (double *) &camera.pos, 2);
}

static Vector *camera_get_scale(PyObject *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &camera.scale, 2, NULL);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int camera_set_scale(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "scale")
    return vector_set(value, (double *) &camera.scale, 2);
}

static int camera_init(PyObject *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", NULL};

    camera.pos.x = 0;
    camera.pos.y = 0;

    camera.scale.x = 1;
    camera.scale.y = 1;

    return PyArg_ParseTupleAndKeywords(args, kwds, "|dd:Camera", kwlist, &camera.pos.x, &camera.pos.y) ? 0 : -1;
}

static PyObject *camera_get_top(PyObject *self, void *closure) {
    return PyFloat_FromDouble(camera.pos.y - window.size.y / 2);
}

static int camera_set_top(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "top")

    double pos = PyFloat_AsDouble(value);
    return ERR(pos) ? -1 : (camera.pos.y = pos + window.size.y / 2, 0);
}

static PyObject *camera_get_left(PyObject *self, void *closure) {
    return PyFloat_FromDouble(camera.pos.x - window.size.x / 2);
}

static int camera_set_left(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "left")

    double pos = PyFloat_AsDouble(value);
    return ERR(pos) ? -1 : (camera.pos.x = pos + window.size.x / 2, 0);
}

static PyObject *camera_get_bottom(PyObject *self, void *closure) {
    return PyFloat_FromDouble(camera.pos.y + window.size.y / 2);
}

static int camera_set_bottom(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "bottom")

    double pos = PyFloat_AsDouble(value);
    return ERR(pos) ? -1 : (camera.pos.y = pos - window.size.y / 2, 0);
}

static PyObject *camera_get_right(PyObject *self, void *closure) {
    return PyFloat_FromDouble(camera.pos.x + window.size.x / 2);
}

static int camera_set_right(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "right")

    double pos = PyFloat_AsDouble(value);
    return ERR(pos) ? -1 : (camera.pos.x = pos - window.size.x / 2, 0);
}

static PyGetSetDef camera_getset[] = {
    {"x", camera_get_x, camera_set_x, "The x position of the camera", NULL},
    {"y", camera_get_y, camera_set_y, "The y position of the camera", NULL},
    {"pos", (getter) camera_get_pos, camera_set_pos, "The position of the camera", NULL},
    {"position", (getter) camera_get_pos, camera_set_pos, "The position of the camera", NULL},
    {"scale", (getter) camera_get_scale, camera_set_scale, "The zoom of the camera", NULL},
    {"zoom", (getter) camera_get_scale, camera_set_scale, "The zoom of the camera", NULL},
    {"top", camera_get_top, camera_set_top, "The top position of the camera", NULL},
    {"left", camera_get_left, camera_set_left, "The left position of the camera", NULL},
    {"bottom", camera_get_bottom, camera_set_bottom, "The bottom position of the camera", NULL},
    {"right", camera_get_right, camera_set_right, "The right position of the camera", NULL},
    {NULL}
};

static PyType_Slot camera_slots[] = {
    {Py_tp_doc, "The view applied to everything on the screen"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_init, camera_init},
    {Py_tp_getset, camera_getset},
    {0}
};

Spec camera_data = {{"Camera", 0, 0, Py_TPFLAGS_DEFAULT, camera_slots}};