#include "main.h"

static int pos(void *unused) {
    return SDL_WarpMouseInWindow(window.sdl, mouse.pos.x + window.size.x / 2, window.size.y / 2 - mouse.pos.y), 0;
}

static PyObject *mouse_get_x(PyObject *self, void *closure) {
    return PyFloat_FromDouble(mouse.pos.x);
}

static int mouse_set_x(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "x")
    return ERR(mouse.pos.x = PyFloat_AsDouble(value)) ? -1 : pos(NULL);
}

static PyObject *mouse_get_y(PyObject *self, void *closure) {
    return PyFloat_FromDouble(mouse.pos.y);
}

static int mouse_set_y(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "y")
    return ERR(mouse.pos.y = PyFloat_AsDouble(value)) ? -1 : pos(NULL);
}

static Vector *mouse_get_pos(PyObject *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &mouse.pos, 2, (int (*)(PyObject *)) pos);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int mouse_set_pos(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "pos")
    return vector_set(value, (double *) &mouse.pos, 2) ? -1 : pos(NULL);
}

static Vector *mouse_get_move(PyObject *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &mouse.move, 2, NULL); // Also provides a setter, which we can ignore

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static PyObject *mouse_get_press(PyObject *self, void *closure) {
    return PyBool_FromLong(mouse.press);
}

static PyObject *mouse_get_release(PyObject *self, void *closure) {
    return PyBool_FromLong(mouse.release);
}

// static int mouse_init(PyObject *self, PyObject *args, PyObject *kwds) {
//     static char *kwlist[] = {"x", "y", NULL};

//     mouse.pos.x = 0;
//     mouse.pos.y = 0;

//     return PyArg_ParseTupleAndKeywords(args, kwds, "|dd:Mouse", kwlist, &mouse.pos.x, &mouse.pos.y) ? pos(NULL) : -1;
// }

static PyObject *mouse_getattro(PyObject *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);

    if (name) {
        Button *item = bsearch(name, mouse.button, mouse.len, sizeof(Button), (int (*)(const void *, const void *)) button_compare);
        return item ? Py_NewRef(item) : PyObject_GenericGetAttr(self, attr);
    }

    return NULL;
}

static PyGetSetDef mouse_getset[] = {
    {"x", mouse_get_x, mouse_set_x, "The x position of the mouse", NULL},
    {"y", mouse_get_y, mouse_set_y, "The y position of the mouse", NULL},
    {"pos", (getter) mouse_get_pos, mouse_set_pos, "The position of the mouse", NULL},
    {"position", (getter) mouse_get_pos, mouse_set_pos, "The position of the mouse", NULL},
    {"move", (getter) mouse_get_move, NULL, "The movement of the mouse", NULL},
    {"press", mouse_get_press, NULL, "A mouse button is pressed", NULL},
    {"release", mouse_get_release, NULL, "A mouse button is released", NULL},
    {NULL}
};

static PyType_Slot mouse_slots[] = {
    {Py_tp_doc, "The input handler for the cursor"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_getattro, mouse_getattro},
    {Py_tp_getset, mouse_getset},
    {0}
};

Spec mouse_data = {{"Mouse", 0, 0, Py_TPFLAGS_DEFAULT, mouse_slots}};