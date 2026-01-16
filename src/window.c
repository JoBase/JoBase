#include "main.h"

static int clear(void *unused) {
    return glClearColor(window.color.x, window.color.y, window.color.z, 1), 0;
}

static int size(void *unused) {
#ifdef __EMSCRIPTEN__
    window.size.x = width();
    window.size.y = height();
#else
    if (!SDL_SetWindowSize(window.sdl, window.size.x, window.size.y))
        return PyErr_SetString(error, SDL_GetError()), -1;
#endif
    return 0;
}

static PyObject *window_get_title(PyObject *self, void *closure) {
    return PyUnicode_FromString(window.title);
}

static int window_set_title(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "title")

    const char *title = PyUnicode_AsUTF8(value);
    INIT(!title)

    if (!SDL_SetWindowTitle(window.sdl, title))
        return PyErr_SetString(error, SDL_GetError()), -1;

    free(window.title);
    return (window.title = strdup(title)) ? 0 : (PyErr_NoMemory(), -1);
}

static PyObject *window_get_red(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.color.x);
}

static int window_set_red(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "red")
    return ERR(window.color.x = PyFloat_AsDouble(value)) ? -1 : clear(NULL);
}

static PyObject *window_get_green(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.color.y);
}

static int window_set_green(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "green")
    return ERR(window.color.y = PyFloat_AsDouble(value)) ? -1 : clear(NULL);
}

static PyObject *window_get_blue(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.color.z);
}

static int window_set_blue(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "blue")
    return ERR(window.color.z = PyFloat_AsDouble(value)) ? -1 : clear(NULL);
}

static Vector *window_get_color(PyObject *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &window.color, 3, (int (*)(PyObject *)) clear);

    if (vect) {
        vect -> names[r] = 'r';
        vect -> names[g] = 'g';
        vect -> names[b] = 'b';
    }

    return vect;
}

static int window_set_color(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "color")
    return vector_set(value, (double *) &window.color, 3) ? -1 : clear(NULL);
}

static PyObject *window_get_width(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.size.x);
}

static int window_set_width(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(window.size.x = PyFloat_AsDouble(value)) ? -1 : size(NULL);
}

static PyObject *window_get_height(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.size.y);
}

static int window_set_height(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "height")
    return ERR(window.size.y = PyFloat_AsDouble(value)) ? -1 : size(NULL);
}

static Vector *window_get_size(PyObject *self, void *closure) {
    Vector *vect = vector_new(NULL, (double *) &window.size, 2, (int (*)(PyObject *)) size);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int window_set_size(PyObject *self, PyObject *value, void *closure) {
    DEL(value, "size")
    return vector_set(value, (double *) &window.size, 2) ? -1 : size(NULL);
}

static PyObject *window_get_resize(PyObject *self, void *closure) {
    return PyBool_FromLong(window.resize);
}

static PyObject *window_get_time(PyObject *self, void *closure) {
    return PyFloat_FromDouble((double) SDL_GetTicks() / 1e3);
}

static PyObject *window_get_top(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.size.y / 2);
}

static PyObject *window_get_left(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.size.x / -2);
}

static PyObject *window_get_bottom(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.size.y / -2);
}

static PyObject *window_get_right(PyObject *self, void *closure) {
    return PyFloat_FromDouble(window.size.x / 2);
}

static int window_init(PyObject *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"title", "width", "height", "color", NULL};

    const char *title = "JoBase";
    PyObject *color = NULL;

    window.color.x = 1;
    window.color.y = 1;
    window.color.z = 1;

    window.size.x = 640;
    window.size.y = 480;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sddO:Window", kwlist, &title, &window.size.x, &window.size.y,
        &color) || vector_set(color, (double *) &window.color, 4))

    if (!(window.title = strdup(title)))
        return PyErr_NoMemory(), -1;

    if (!SDL_SetWindowTitle(window.sdl, title))
        return PyErr_SetString(error, SDL_GetError()), -1;

    return clear(NULL), size(NULL);
}

static PyObject *window_close(PyObject *self, PyObject *args) {
    SDL_Event event = {.type = SDL_EVENT_QUIT};
    return SDL_PushEvent(&event) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyObject *window_maximize(PyObject *self, PyObject *args) {
    return SDL_MaximizeWindow(window.sdl) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyObject *window_minimize(PyObject *self, PyObject *args) {
    return SDL_MinimizeWindow(window.sdl) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyObject *window_restore(PyObject *self, PyObject *args) {
    return SDL_RestoreWindow(window.sdl) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyMethodDef window_methods[] = {
    {"close", window_close, METH_STATIC | METH_NOARGS, "Close the window and terminate the game loop"},
    {"maximize", window_maximize, METH_STATIC | METH_NOARGS, "Maximize the window"},
    {"minimize", window_minimize, METH_STATIC | METH_NOARGS, "Minimize the window"},
    {"restore", window_restore, METH_STATIC | METH_NOARGS, "Restore the window from maximized or minimized"},
    {NULL}
};

static PyGetSetDef window_getset[] = {
    {"title", window_get_title, window_set_title, "The caption of the window", NULL},
    {"red", window_get_red, window_set_red, "Red value of the window's background color", NULL},
    {"green", window_get_green, window_set_green, "Green value of the window's background colorw", NULL},
    {"blue", window_get_blue, window_set_blue, "Blue value of the window's background color", NULL},
    {"color", (getter) window_get_color, window_set_color, "The background color of the window", NULL},
    {"width", window_get_width, window_set_width, "The width of the window", NULL},
    {"height", window_get_height, window_set_height, "The height of the window", NULL},
    {"size", (getter) window_get_size, window_set_size, "The dimensions of the window", NULL},
    {"resize", window_get_resize, NULL, "Determine whether window has been resized", NULL},
    {"time", window_get_time, NULL, "The number of seconds since the program started", NULL},
    {"top", window_get_top, NULL, "The top position of the window", NULL},
    {"left", window_get_left, NULL, "The left position of the window", NULL},
    {"bottom", window_get_bottom, NULL, "The bottom position of the window", NULL},
    {"right", window_get_right, NULL, "The right position of the window", NULL},
    {NULL}
};

static PyType_Slot window_slots[] = {
    {Py_tp_doc, "Main object for managing the window"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_init, window_init},
    {Py_tp_methods, window_methods},
    {Py_tp_getset, window_getset},
    {0}
};

Spec window_data = {{"Window", 0, 0, Py_TPFLAGS_DEFAULT, window_slots}};