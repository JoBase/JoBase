#include <main.h>

static PyObject *Key_getPress(Key *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> press);
}

static PyObject *Key_getRelease(Key *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> release);
}

static PyObject *Key_getRepeat(Key *self, void *Py_UNUSED(closure)) {
    return PyBool_FromLong(self -> repeat);
}

static PyObject *Key_getHold(Key *self, void *Py_UNUSED(closure)) {
    FOR(uint16_t, GLFW_KEY_LAST)
        if (self -> keys[i].hold) Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static PyObject *Key_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    key = (Key *) type -> tp_alloc(type, 0);
    Set *e = key -> keys;

    e[GLFW_KEY_SPACE].key = "space";
    e[GLFW_KEY_APOSTROPHE].key = "apostrophe";
    e[GLFW_KEY_COMMA].key = "comma";
    e[GLFW_KEY_MINUS].key = "minus";
    e[GLFW_KEY_PERIOD].key = "period";
    e[GLFW_KEY_SLASH].key = "slash";
    e[GLFW_KEY_0].key = "_0";
    e[GLFW_KEY_1].key = "_1";
    e[GLFW_KEY_2].key = "_2";
    e[GLFW_KEY_3].key = "_3";
    e[GLFW_KEY_4].key = "_4";
    e[GLFW_KEY_5].key = "_5";
    e[GLFW_KEY_6].key = "_6";
    e[GLFW_KEY_7].key = "_7";
    e[GLFW_KEY_8].key = "_8";
    e[GLFW_KEY_9].key = "_9";
    e[GLFW_KEY_SEMICOLON].key = "semicolon";
    e[GLFW_KEY_EQUAL].key = "equal";
    e[GLFW_KEY_A].key = "a";
    e[GLFW_KEY_B].key = "b";
    e[GLFW_KEY_C].key = "c";
    e[GLFW_KEY_D].key = "d";
    e[GLFW_KEY_E].key = "e";
    e[GLFW_KEY_F].key = "f";
    e[GLFW_KEY_G].key = "g";
    e[GLFW_KEY_H].key = "h";
    e[GLFW_KEY_I].key = "i";
    e[GLFW_KEY_J].key = "j";
    e[GLFW_KEY_K].key = "k";
    e[GLFW_KEY_L].key = "l";
    e[GLFW_KEY_M].key = "m";
    e[GLFW_KEY_N].key = "n";
    e[GLFW_KEY_O].key = "o";
    e[GLFW_KEY_P].key = "p";
    e[GLFW_KEY_Q].key = "q";
    e[GLFW_KEY_R].key = "r";
    e[GLFW_KEY_S].key = "s";
    e[GLFW_KEY_T].key = "t";
    e[GLFW_KEY_U].key = "u";
    e[GLFW_KEY_V].key = "v";
    e[GLFW_KEY_W].key = "w";
    e[GLFW_KEY_X].key = "x";
    e[GLFW_KEY_Y].key = "y";
    e[GLFW_KEY_Z].key = "z";
    e[GLFW_KEY_LEFT_BRACKET].key = "left_bracket";
    e[GLFW_KEY_BACKSLASH].key = "backslash";
    e[GLFW_KEY_RIGHT_BRACKET].key = "right_bracket";
    e[GLFW_KEY_GRAVE_ACCENT].key = "backquote";
    e[GLFW_KEY_ESCAPE].key = "escape";
    e[GLFW_KEY_ENTER].key = "enter";
    e[GLFW_KEY_TAB].key = "tab";
    e[GLFW_KEY_BACKSPACE].key = "backspace";
    e[GLFW_KEY_INSERT].key = "insert";
    e[GLFW_KEY_DELETE].key = "delete";
    e[GLFW_KEY_RIGHT].key = "right";
    e[GLFW_KEY_LEFT].key = "left";
    e[GLFW_KEY_DOWN].key = "down";
    e[GLFW_KEY_UP].key = "up";
    e[GLFW_KEY_PAGE_UP].key = "page_up";
    e[GLFW_KEY_PAGE_DOWN].key = "page_down";
    e[GLFW_KEY_HOME].key = "home";
    e[GLFW_KEY_END].key = "end";
    e[GLFW_KEY_CAPS_LOCK].key = "caps_lock";
    e[GLFW_KEY_SCROLL_LOCK].key = "scroll_lock";
    e[GLFW_KEY_NUM_LOCK].key = "num_lock";
    e[GLFW_KEY_PRINT_SCREEN].key = "print_screen";
    e[GLFW_KEY_PAUSE].key = "pause";
    e[GLFW_KEY_F1].key = "f1";
    e[GLFW_KEY_F2].key = "f2";
    e[GLFW_KEY_F3].key = "f3";
    e[GLFW_KEY_F4].key = "f4";
    e[GLFW_KEY_F5].key = "f5";
    e[GLFW_KEY_F6].key = "f6";
    e[GLFW_KEY_F7].key = "f7";
    e[GLFW_KEY_F8].key = "f8";
    e[GLFW_KEY_F9].key = "f9";
    e[GLFW_KEY_F10].key = "f10";
    e[GLFW_KEY_F11].key = "f11";
    e[GLFW_KEY_F12].key = "f12";
    e[GLFW_KEY_F13].key = "f13";
    e[GLFW_KEY_F14].key = "f14";
    e[GLFW_KEY_F15].key = "f15";
    e[GLFW_KEY_F16].key = "f16";
    e[GLFW_KEY_F17].key = "f17";
    e[GLFW_KEY_F18].key = "f18";
    e[GLFW_KEY_F19].key = "f19";
    e[GLFW_KEY_F20].key = "f20";
    e[GLFW_KEY_F21].key = "f21";
    e[GLFW_KEY_F22].key = "f22";
    e[GLFW_KEY_F23].key = "f23";
    e[GLFW_KEY_F24].key = "f24";
    e[GLFW_KEY_F25].key = "f25";
    e[GLFW_KEY_KP_0].key = "pad_0";
    e[GLFW_KEY_KP_1].key = "pad_1";
    e[GLFW_KEY_KP_2].key = "pad_2";
    e[GLFW_KEY_KP_3].key = "pad_3";
    e[GLFW_KEY_KP_4].key = "pad_4";
    e[GLFW_KEY_KP_5].key = "pad_5";
    e[GLFW_KEY_KP_6].key = "pad_6";
    e[GLFW_KEY_KP_7].key = "pad_7";
    e[GLFW_KEY_KP_8].key = "pad_8";
    e[GLFW_KEY_KP_9].key = "pad_9";
    e[GLFW_KEY_KP_DECIMAL].key = "pad_decimal";
    e[GLFW_KEY_KP_DIVIDE].key = "pad_divide";
    e[GLFW_KEY_KP_MULTIPLY].key = "pad_multiply";
    e[GLFW_KEY_KP_SUBTRACT].key = "pad_subtract";
    e[GLFW_KEY_KP_ADD].key = "pad_add";
    e[GLFW_KEY_KP_ENTER].key = "pad_enter";
    e[GLFW_KEY_KP_EQUAL].key = "pad_equal";
    e[GLFW_KEY_LEFT_SHIFT].key = "left_shift";
    e[GLFW_KEY_LEFT_CONTROL].key = "left_ctrl";
    e[GLFW_KEY_LEFT_ALT].key = "left_alt";
    e[GLFW_KEY_LEFT_SUPER].key = "left_super";
    e[GLFW_KEY_RIGHT_SHIFT].key = "right_shift";
    e[GLFW_KEY_RIGHT_CONTROL].key = "right_ctrl";
    e[GLFW_KEY_RIGHT_ALT].key = "right_alt";
    e[GLFW_KEY_RIGHT_SUPER].key = "right_super";
    e[GLFW_KEY_MENU].key = "menu";

    Py_XINCREF(key);
    return (PyObject *) key;
}

static PyObject *Key_getattro(Key *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);
    if (!name) return NULL;
    
    FOR(uint16_t, GLFW_KEY_LAST)
        if (self -> keys[i].key && !strcmp(self -> keys[i].key, name))
            return (PyObject *) buttonNew(&self -> keys[i]);

    return PyObject_GenericGetAttr((PyObject *) self, attr);
}

static PyGetSetDef KeyGetSetters[] = {
    {"press", (getter) Key_getPress, NULL, "a key is pressed", NULL},
    {"release", (getter) Key_getRelease, NULL, "a key is released", NULL},
    {"repeat", (getter) Key_getRepeat, NULL, "triggered when a key is held down", NULL},
    {"hold", (getter) Key_getHold, NULL, "a key is held down", NULL},
    {NULL}
};

PyTypeObject KeyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Key",
    .tp_doc = "the keyboard input handler",
    .tp_basicsize = sizeof(Key),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Key_new,
    .tp_getattro = (getattrofunc) Key_getattro,
    .tp_getset = KeyGetSetters
};