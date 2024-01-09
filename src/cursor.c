#include <main.h>

static int pos(Cursor *self) {
    return glfwSetCursorPos(window -> glfw, self -> pos.x + window -> size.x / 2, window -> size.y / 2 - self -> pos.y), 0;
}

static PyObject *Cursor_get_x(Cursor *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.x);
}

static int Cursor_set_x(Cursor *self, PyObject *value, void *closure) {
    DEL(value, "x")
    return ERR(self -> pos.x = PyFloat_AsDouble(value)) ? -1 : pos(self);
}

static PyObject *Cursor_get_y(Cursor *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.y);
}

static int Cursor_set_y(Cursor *self, PyObject *value, void *closure) {
    DEL(value, "y")
    return ERR(self -> pos.y = PyFloat_AsDouble(value)) ? -1 : pos(self);
}

static Vector *Cursor_get_pos(Cursor *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> pos, 2, (set) pos);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Cursor_set_pos(Cursor *self, PyObject *value, void *closure) {
    DEL(value, "pos")
    return Vector_set(value, (vec) &self -> pos, 2) ? -1 : pos(self);
}

static PyObject *Cursor_get_move(Cursor *self, void *closure) {
    return PyBool_FromLong(self -> move);
}

static PyObject *Cursor_get_enter(Cursor *self, void *closure) {
    return PyBool_FromLong(self -> enter);
}

static PyObject *Cursor_get_leave(Cursor *self, void *closure) {
    return PyBool_FromLong(self -> leave);
}

static PyObject *Cursor_get_press(Cursor *self, void *closure) {
    return PyBool_FromLong(self -> press);
}

static PyObject *Cursor_get_release(Cursor *self, void *closure) {
    return PyBool_FromLong(self -> release);
}

static PyObject *Cursor_get_hold(Cursor *self, void *closure) {
    for (uint8_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; i ++)
        if (self -> buttons[i].hold) Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static Cursor *Cursor_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Cursor *self = (Cursor *) type -> tp_alloc(type, 0);

    if (self) {
        Set *button = self -> buttons;

        button[GLFW_MOUSE_BUTTON_LEFT].name = "left";
        button[GLFW_MOUSE_BUTTON_RIGHT].name = "right";
        button[GLFW_MOUSE_BUTTON_MIDDLE].name = "middle";
        button[GLFW_MOUSE_BUTTON_4].name = "_4";
        button[GLFW_MOUSE_BUTTON_5].name = "_5";
        button[GLFW_MOUSE_BUTTON_6].name = "_6";
        button[GLFW_MOUSE_BUTTON_7].name = "_7";
        button[GLFW_MOUSE_BUTTON_8].name = "_8";
    }

    return self;
}

static PyObject *Cursor_getattro(Cursor *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);

    if (name) {
        for (uint8_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; i ++)
            if (!strcmp(self -> buttons[i].name, name))
                return (PyObject *) Button_new(&self -> buttons[i]);

        return PyObject_GenericGetAttr((PyObject *) self, attr);
    }

    return NULL;
}

static PyGetSetDef Cursor_getset[] = {
    {"x", (getter) Cursor_get_x, (setter) Cursor_set_x, "x position of the cursor", NULL},
    {"y", (getter) Cursor_get_y, (setter) Cursor_set_y, "y position of the cursor", NULL},
    {"position", (getter) Cursor_get_pos, (setter) Cursor_set_pos, "position of the cursor", NULL},
    {"pos", (getter) Cursor_get_pos, (setter) Cursor_set_pos, "position of the cursor", NULL},
    {"move", (getter) Cursor_get_move, NULL, "the cursor has moved", NULL},
    {"enter", (getter) Cursor_get_enter, NULL, "the cursor has entered the window", NULL},
    {"leave", (getter) Cursor_get_leave, NULL, "the cursor has left the window", NULL},
    {"press", (getter) Cursor_get_press, NULL, "a mouse button is pressed", NULL},
    {"release", (getter) Cursor_get_release, NULL, "a mouse button is released", NULL},
    {"hold", (getter) Cursor_get_hold, NULL, "a mouse button is held down", NULL},
    {NULL}
};

static PyMethodDef Cursor_methods[] = {
    {"collides_with", (PyCFunction) Base_collide, METH_O, "check if cursor and object collide"},
    {"collide", (PyCFunction) Base_collide, METH_O, "check if cursor and object collide"},
    {NULL}
};

PyTypeObject CursorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Cursor",
    .tp_doc = "input handler for the mouse pointer",
    .tp_basicsize = sizeof(Cursor),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc) Cursor_new,
    .tp_getattro = (getattrofunc) Cursor_getattro,
    .tp_methods = Cursor_methods,
    .tp_getset = Cursor_getset
};