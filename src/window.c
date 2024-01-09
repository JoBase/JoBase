#include <main.h>

static int clear(Window *self) {
    return glClearColor(self -> color.r, self -> color.g, self -> color.b, 1), 0;
}

static int size(Window *self) {
    return glfwSetWindowSize(self -> glfw, self -> size.x, self -> size.y), 0;
}

static void resize(Window *self) {
    self -> size.x = 640;
    self -> size.y = 480;

#ifdef __EMSCRIPTEN__
    self -> size.x = width();
    self -> size.y = height();
#endif
}

static void cursor_pos_callback(GLFWwindow *glfw, double px, double py) {
    cursor -> move = true;
    cursor -> pos.x = px - window -> size.x / 2;
    cursor -> pos.y = window -> size.y / 2 - py;
}

static void window_size_callback(GLFWwindow *glfw, int width, int height) {
    window -> resize = true;
    window -> size.x = width;
    window -> size.y = height;
}

static void framebuffer_size_callback(GLFWwindow *glfw, int width, int height) {
    glViewport(0, 0, width, height);
}

static void cursor_enter_callback(GLFWwindow *glfw, int entered) {
    entered ? (cursor -> enter = true) : (cursor -> leave = true);
}

static void mouse_button_callback(GLFWwindow *glfw, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        cursor -> press = true;
        cursor -> buttons[button].press = true;
        cursor -> buttons[button].hold = true;
    }

    else if (action == GLFW_RELEASE) {
        cursor -> release = true;
        cursor -> buttons[button].release = true;
        cursor -> buttons[button].hold = false;
    }
}

static void key_callback(GLFWwindow *glfw, int type, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        key -> press = true;
        key -> keys[type].press = true;
        key -> keys[type].hold = true;
    }

    else if (action == GLFW_RELEASE) {
        key -> release = true;
        key -> keys[type].release = true;
        key -> keys[type].hold = false;
    }

    else if (action == GLFW_REPEAT) {
        key -> repeat = true;
        key -> keys[type].repeat = true;
    }
}

static PyObject *Window_get_title(Window *self, void *closure) {
    return PyUnicode_FromString(self -> title);
}

static int Window_set_title(Window *self, PyObject *value, void *closure) {
    DEL(value, "title")

    const char *title = PyUnicode_AsUTF8(value);
    INIT(!title)

    glfwSetWindowTitle(self -> glfw, title);
    free(self -> title);

    return self -> title = strdup(title), 0;
}

static PyObject *Window_get_red(Window *self, void *closure) {
    return PyFloat_FromDouble(self -> color.r);
}

static int Window_set_red(Window *self, PyObject *value, void *closure) {
    DEL(value, "red")
    return ERR(self -> color.r = PyFloat_AsDouble(value)) ? -1 : clear(self);
}

static PyObject *Window_get_green(Window *self, void *closure) {
    return PyFloat_FromDouble(self -> color.g);
}

static int Window_set_green(Window *self, PyObject *value, void *closure) {
    DEL(value, "green")
    return ERR(self -> color.g = PyFloat_AsDouble(value)) ? -1 : clear(self);
}

static PyObject *Window_get_blue(Window *self, void *closure) {
    return PyFloat_FromDouble(self -> color.b);
}

static int Window_set_blue(Window *self, PyObject *value, void *closure) {
    DEL(value, "blue")
    return ERR(self -> color.b = PyFloat_AsDouble(value)) ? -1 : clear(self);
}

static Vector *Window_get_color(Window *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> color, 3, (set) clear);

    if (vector) {
        vector -> names[r] = 'r';
        vector -> names[g] = 'g';
        vector -> names[b] = 'b';
    }

    return vector;
}

static int Window_set_color(Window *self, PyObject *value, void *closure) {
    DEL(value, "color")
    return Vector_set(value, (vec) &self -> color, 3) ? -1 : clear(self);
}

static PyObject *Window_get_width(Window *self, void *closure) {
    return PyFloat_FromDouble(self -> size.x);
}

static int Window_set_width(Window *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(self -> size.x = PyFloat_AsDouble(value)) ? -1 : size(self);
}

static PyObject *Window_get_height(Window *self, void *closure) {
    return PyFloat_FromDouble(self -> size.y);
}

static int Window_set_height(Window *self, PyObject *value, void *closure) {
    DEL(value, "height")
    return ERR(self -> size.y = PyFloat_AsDouble(value)) ? -1 : size(self);
}

static Vector *Window_get_size(Window *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> size, 2, (set) size);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Window_set_size(Window *self, PyObject *value, void *closure) {
    DEL(value, "size")
    return Vector_set(value, (vec) &self -> size, 2) ? -1 : size(self);
}

static PyObject *Window_get_resize(Window *self, void *closure) {
    return PyBool_FromLong(self -> resize);
}

static PyObject *Window_close(Window *self, PyObject *args) {
    glfwSetWindowShouldClose(self -> glfw, GLFW_TRUE);
    Py_RETURN_NONE;
}

static PyObject *Window_maximize(Window *self, PyObject *args) {
    glfwMaximizeWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyObject *Window_minimize(Window *self, PyObject *args) {
    glfwIconifyWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyObject *Window_restore(Window *self, PyObject *args) {
    glfwRestoreWindow(self -> glfw);
    Py_RETURN_NONE;
}

static PyObject *Window_focus(Window *self, PyObject *args) {
    glfwFocusWindow(self -> glfw);
    Py_RETURN_NONE;
}

static Window *Window_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Window *self = (Window *) type -> tp_alloc(type, 0);

    if (self) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_ALPHA_BITS, 0);
        glfwWindowHint(GLFW_SAMPLES, 4);
        resize(self);

        if (!(self -> glfw = glfwCreateWindow(self -> size.x, self -> size.y, "JoBase", NULL, NULL))) {
            PyErr_SetString(PyExc_OSError, "failed to create the window");
            return NULL;
        }

        glfwMakeContextCurrent(self -> glfw);
        glfwSetCursorPosCallback(self -> glfw, cursor_pos_callback);
        glfwSetWindowSizeCallback(self -> glfw, window_size_callback);
        glfwSetFramebufferSizeCallback(self -> glfw, framebuffer_size_callback);
        glfwSetCursorEnterCallback(self -> glfw, cursor_enter_callback);
        glfwSetMouseButtonCallback(self -> glfw, mouse_button_callback);
        glfwSetKeyCallback(self -> glfw, key_callback);
        glfwSwapInterval(1);

#ifndef __EMSCRIPTEN__
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            PyErr_SetString(PyExc_OSError, "failed to load OpenGL");
            return NULL;
        }
#endif
    }

    return self;
}

static int Window_init(Window *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"title", "width", "height", "color", NULL};
    const char *title = "JoBase";

    PyObject *color = NULL;
    resize(self);

    self -> color.r = 1;
    self -> color.g = 1;
    self -> color.b = 1;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sddO:Window", kwlist, &title, &self -> size.x,
        &self -> size.y, &color) || Vector_set(color, (vec) &self -> color, 3))

    self -> title = strdup(title);
    glfwSetWindowTitle(self -> glfw, title);

    return size(self), clear(self);
}

static void Window_dealloc(Window *self) {
    free(self -> title);
    WindowType.tp_free(self);
}

static PyGetSetDef Window_getset[] = {
    {"title", (getter) Window_get_title, (setter) Window_set_title, "the window's caption", NULL},
    {"red", (getter) Window_get_red, (setter) Window_set_red, "red value of the window's background color", NULL},
    {"green", (getter) Window_get_green, (setter) Window_set_green, "green value of the window's background colorw", NULL},
    {"blue", (getter) Window_get_blue, (setter) Window_set_blue, "blue value of the window's background color", NULL},
    {"color", (getter) Window_get_color, (setter) Window_set_color, "background color of the window", NULL},
    {"width", (getter) Window_get_width, (setter) Window_set_width, "width of the window", NULL},
    {"height", (getter) Window_get_height, (setter) Window_set_height, "height of the window", NULL},
    {"size", (getter) Window_get_size, (setter) Window_set_size, "dimensions of the window", NULL},
    {"resize", (getter) Window_get_resize, NULL, "determine whether window has been resized", NULL},
    {NULL}
};

static PyMethodDef Window_methods[] = {
    {"close", (PyCFunction) Window_close, METH_NOARGS, "close the window and terminate the game loop"},
    {"maximize", (PyCFunction) Window_maximize, METH_NOARGS, "maximize the window"},
    {"minimize", (PyCFunction) Window_minimize, METH_NOARGS, "minimize the window"},
    {"restore", (PyCFunction) Window_restore, METH_NOARGS, "restore the window from maximized or minimized"},
    {"focus", (PyCFunction) Window_focus, METH_NOARGS, "bring the window to the front"},
    {NULL}
};

PyTypeObject WindowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Window",
    .tp_doc = "the main window where everything is rendered",
    .tp_basicsize = sizeof(Window),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc) Window_new,
    .tp_init = (initproc) Window_init,
    .tp_dealloc = (destructor) Window_dealloc,
    .tp_methods = Window_methods,
    .tp_getset = Window_getset
};