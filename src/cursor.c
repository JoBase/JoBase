#include <main.h>

static PyObject *Cursor_getX(Cursor *Py_UNUSED(self), void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cursorPos()[x]);
}

static int Cursor_setX(Cursor *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double pos = PyFloat_AsDouble(value);
    if (ERR(pos)) return -1;

    start();
    glfwSetCursorPos(window -> glfw, pos + windowSize()[x] / 2, windowSize()[y]);
    return end(), 0;
}

static PyObject *Cursor_getY(Cursor *Py_UNUSED(self), void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(cursorPos()[1]);
}

static int Cursor_setY(Cursor *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double pos = PyFloat_AsDouble(value);
    if (ERR(pos)) return -1;

    start();
    glfwSetCursorPos(window -> glfw, cursorPos()[x], windowSize()[y] / 2 - pos);
    return end(), 0;
}

static PyObject *Cursor_getPos(Cursor *self, void *Py_UNUSED(closure)) {
    Vector *pos = vectorNew((PyObject *) self, (Getter) cursorPos, 2);

    pos -> data[x].name = "x";
    pos -> data[y].name = "y";

    return (PyObject *) pos;
}

static int Cursor_setPos(Cursor *Py_UNUSED(self), PyObject *value, void *Py_UNUSED(closure)) {
    vec pos = cursorPos();
    vec size = windowSize();

    if (vectorSet(value, pos, 2))
        return -1;

    start();
    glfwSetCursorPos(window -> glfw, pos[x] + size[x] / 2, size[y] / 2 - pos[y]);
    return end(), 0;
}

static PyObject *Cursor_getMove(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> move);
}

static PyObject *Cursor_getEnter(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> enter);
}

static PyObject *Cursor_getLeave(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> leave);
}

static PyObject *Cursor_getPress(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> press);
}

static PyObject *Cursor_getRelease(Cursor *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> release);
}

static PyObject *Cursor_getHold(Cursor *self, void *Py_UNUSED(closure)) {
    FOR(uint8_t, GLFW_MOUSE_BUTTON_LAST)
        if (self -> buttons[i].hold) Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *Cursor_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    cursor = (Cursor *) type -> tp_alloc(type, 0);
    Set *e = cursor -> buttons;

    e[GLFW_MOUSE_BUTTON_LEFT].key = "left";
    e[GLFW_MOUSE_BUTTON_RIGHT].key = "right";
    e[GLFW_MOUSE_BUTTON_MIDDLE].key = "middle";
    e[GLFW_MOUSE_BUTTON_4].key = "_4";
    e[GLFW_MOUSE_BUTTON_5].key = "_5";
    e[GLFW_MOUSE_BUTTON_6].key = "_6";
    e[GLFW_MOUSE_BUTTON_7].key = "_7";
    e[GLFW_MOUSE_BUTTON_8].key = "_8";

    Py_XINCREF(cursor);
    return (PyObject *) cursor;
}

static PyObject *Cursor_getattro(Cursor *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return NULL;
    
    FOR(uint8_t, GLFW_MOUSE_BUTTON_LAST)
        if (self -> buttons[i].key && !strcmp(self -> buttons[i].key, name))
            return (PyObject *) buttonNew(&self -> buttons[i]);

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static PyGetSetDef CursorGetSetters[] = {
    {"x", (getter) Cursor_getX, (setter) Cursor_setX, "x position of the cursor", NULL},
    {"y", (getter) Cursor_getY, (setter) Cursor_setY, "y position of the cursor", NULL},
    {"position", (getter) Cursor_getPos, (setter) Cursor_setPos, "position of the cursor", NULL},
    {"pos", (getter) Cursor_getPos, (setter) Cursor_setPos, "position of the cursor", NULL},
    {"move", (getter) Cursor_getMove, NULL, "the cursor has moved", NULL},
    {"enter", (getter) Cursor_getEnter, NULL, "the cursor has entered the window", NULL},
    {"leave", (getter) Cursor_getLeave, NULL, "the cursor has left the window", NULL},
    {"press", (getter) Cursor_getPress, NULL, "a mouse button is pressed", NULL},
    {"release", (getter) Cursor_getRelease, NULL, "a mouse button is released", NULL},
    {"hold", (getter) Cursor_getHold, NULL, "a mouse button is held down", NULL},
    {NULL}
};

static PyMethodDef CursorMethods[] = {
    {"collides_with", (PyCFunction) collide, METH_O, "check if the cursor collides with another object"},
    {"collide", (PyCFunction) collide, METH_O, "check if the cursor collides with another object"},
    {NULL}
};

vec cursorPos() {
    static vec2 pos;
    glfwGetCursorPos(window -> glfw, &pos[x], &pos[y]);

    vec size = windowSize();
    pos[x] -= size[x] / 2;
    pos[y] = size[y] / 2 - pos[y];

    return pos;
}

PyTypeObject CursorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Cursor",
    .tp_doc = "the cursor or mouse input handler",
    .tp_basicsize = sizeof(Cursor),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Cursor_new,
    .tp_getattro = (getattrofunc) Cursor_getattro,
    .tp_getset = CursorGetSetters,
    .tp_methods = CursorMethods
};