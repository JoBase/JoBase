#include <main.h>

static PyObject *Key_get_press(Key *self, void *closure) {
    return PyBool_FromLong(self -> press);
}

static PyObject *Key_get_release(Key *self, void *closure) {
    return PyBool_FromLong(self -> release);
}

static PyObject *Key_get_repeat(Key *self, void *closure) {
    return PyBool_FromLong(self -> repeat);
}

static PyObject *Key_get_hold(Key *self, void *closure) {
    for (uint16_t i = 0; i < GLFW_KEY_LAST; i ++)
        if (self -> keys[i].hold) Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

static Key *Key_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Key *self = (Key *) type -> tp_alloc(type, 0);

    if (self) {
        Set *key = self -> keys;

        key[GLFW_KEY_SPACE].name = "space";
        key[GLFW_KEY_APOSTROPHE].name = "apostrophe";
        key[GLFW_KEY_COMMA].name = "comma";
        key[GLFW_KEY_MINUS].name = "minus";
        key[GLFW_KEY_PERIOD].name = "period";
        key[GLFW_KEY_SLASH].name = "slash";
        key[GLFW_KEY_0].name = "_0";
        key[GLFW_KEY_1].name = "_1";
        key[GLFW_KEY_2].name = "_2";
        key[GLFW_KEY_3].name = "_3";
        key[GLFW_KEY_4].name = "_4";
        key[GLFW_KEY_5].name = "_5";
        key[GLFW_KEY_6].name = "_6";
        key[GLFW_KEY_7].name = "_7";
        key[GLFW_KEY_8].name = "_8";
        key[GLFW_KEY_9].name = "_9";
        key[GLFW_KEY_SEMICOLON].name = "semicolon";
        key[GLFW_KEY_EQUAL].name = "equal";
        key[GLFW_KEY_A].name = "a";
        key[GLFW_KEY_B].name = "b";
        key[GLFW_KEY_C].name = "c";
        key[GLFW_KEY_D].name = "d";
        key[GLFW_KEY_E].name = "e";
        key[GLFW_KEY_F].name = "f";
        key[GLFW_KEY_G].name = "g";
        key[GLFW_KEY_H].name = "h";
        key[GLFW_KEY_I].name = "i";
        key[GLFW_KEY_J].name = "j";
        key[GLFW_KEY_K].name = "k";
        key[GLFW_KEY_L].name = "l";
        key[GLFW_KEY_M].name = "m";
        key[GLFW_KEY_N].name = "n";
        key[GLFW_KEY_O].name = "o";
        key[GLFW_KEY_P].name = "p";
        key[GLFW_KEY_Q].name = "q";
        key[GLFW_KEY_R].name = "r";
        key[GLFW_KEY_S].name = "s";
        key[GLFW_KEY_T].name = "t";
        key[GLFW_KEY_U].name = "u";
        key[GLFW_KEY_V].name = "v";
        key[GLFW_KEY_W].name = "w";
        key[GLFW_KEY_X].name = "x";
        key[GLFW_KEY_Y].name = "y";
        key[GLFW_KEY_Z].name = "z";
        key[GLFW_KEY_LEFT_BRACKET].name = "left_bracket";
        key[GLFW_KEY_BACKSLASH].name = "backslash";
        key[GLFW_KEY_RIGHT_BRACKET].name = "right_bracket";
        key[GLFW_KEY_GRAVE_ACCENT].name = "backquote";
        key[GLFW_KEY_ESCAPE].name = "escape";
        key[GLFW_KEY_ENTER].name = "enter";
        key[GLFW_KEY_TAB].name = "tab";
        key[GLFW_KEY_BACKSPACE].name = "backspace";
        key[GLFW_KEY_INSERT].name = "insert";
        key[GLFW_KEY_DELETE].name = "delete";
        key[GLFW_KEY_RIGHT].name = "right";
        key[GLFW_KEY_LEFT].name = "left";
        key[GLFW_KEY_DOWN].name = "down";
        key[GLFW_KEY_UP].name = "up";
        key[GLFW_KEY_PAGE_UP].name = "page_up";
        key[GLFW_KEY_PAGE_DOWN].name = "page_down";
        key[GLFW_KEY_HOME].name = "home";
        key[GLFW_KEY_END].name = "end";
        key[GLFW_KEY_CAPS_LOCK].name = "caps_lock";
        key[GLFW_KEY_SCROLL_LOCK].name = "scroll_lock";
        key[GLFW_KEY_NUM_LOCK].name = "num_lock";
        key[GLFW_KEY_PRINT_SCREEN].name = "print_screen";
        key[GLFW_KEY_PAUSE].name = "pause";
        key[GLFW_KEY_F1].name = "f1";
        key[GLFW_KEY_F2].name = "f2";
        key[GLFW_KEY_F3].name = "f3";
        key[GLFW_KEY_F4].name = "f4";
        key[GLFW_KEY_F5].name = "f5";
        key[GLFW_KEY_F6].name = "f6";
        key[GLFW_KEY_F7].name = "f7";
        key[GLFW_KEY_F8].name = "f8";
        key[GLFW_KEY_F9].name = "f9";
        key[GLFW_KEY_F10].name = "f10";
        key[GLFW_KEY_F11].name = "f11";
        key[GLFW_KEY_F12].name = "f12";
        key[GLFW_KEY_F13].name = "f13";
        key[GLFW_KEY_F14].name = "f14";
        key[GLFW_KEY_F15].name = "f15";
        key[GLFW_KEY_F16].name = "f16";
        key[GLFW_KEY_F17].name = "f17";
        key[GLFW_KEY_F18].name = "f18";
        key[GLFW_KEY_F19].name = "f19";
        key[GLFW_KEY_F20].name = "f20";
        key[GLFW_KEY_F21].name = "f21";
        key[GLFW_KEY_F22].name = "f22";
        key[GLFW_KEY_F23].name = "f23";
        key[GLFW_KEY_F24].name = "f24";
        key[GLFW_KEY_F25].name = "f25";
        key[GLFW_KEY_KP_0].name = "pad_0";
        key[GLFW_KEY_KP_1].name = "pad_1";
        key[GLFW_KEY_KP_2].name = "pad_2";
        key[GLFW_KEY_KP_3].name = "pad_3";
        key[GLFW_KEY_KP_4].name = "pad_4";
        key[GLFW_KEY_KP_5].name = "pad_5";
        key[GLFW_KEY_KP_6].name = "pad_6";
        key[GLFW_KEY_KP_7].name = "pad_7";
        key[GLFW_KEY_KP_8].name = "pad_8";
        key[GLFW_KEY_KP_9].name = "pad_9";
        key[GLFW_KEY_KP_DECIMAL].name = "pad_decimal";
        key[GLFW_KEY_KP_DIVIDE].name = "pad_divide";
        key[GLFW_KEY_KP_MULTIPLY].name = "pad_multiply";
        key[GLFW_KEY_KP_SUBTRACT].name = "pad_subtract";
        key[GLFW_KEY_KP_ADD].name = "pad_add";
        key[GLFW_KEY_KP_ENTER].name = "pad_enter";
        key[GLFW_KEY_KP_EQUAL].name = "pad_equal";
        key[GLFW_KEY_LEFT_SHIFT].name = "left_shift";
        key[GLFW_KEY_LEFT_CONTROL].name = "left_ctrl";
        key[GLFW_KEY_LEFT_ALT].name = "left_alt";
        key[GLFW_KEY_LEFT_SUPER].name = "left_super";
        key[GLFW_KEY_RIGHT_SHIFT].name = "right_shift";
        key[GLFW_KEY_RIGHT_CONTROL].name = "right_ctrl";
        key[GLFW_KEY_RIGHT_ALT].name = "right_alt";
        key[GLFW_KEY_RIGHT_SUPER].name = "right_super";
        key[GLFW_KEY_MENU].name = "menu";
    }

    return self;
}

static PyObject *Key_getattro(Key *self, PyObject *attr) {
    const char *name = PyUnicode_AsUTF8(attr);

    if (name) {
        for (uint16_t i = 0; i < GLFW_KEY_LAST; i ++)
            if (self -> keys[i].name && !strcmp(self -> keys[i].name, name))
                return (PyObject *) Button_new(&self -> keys[i]);

        return PyObject_GenericGetAttr((PyObject *) self, attr);
    }

    return NULL;
}

static PyGetSetDef Key_getset[] = {
    {"press", (getter) Key_get_press, NULL, "a key is pressed", NULL},
    {"release", (getter) Key_get_release, NULL, "a key is released", NULL},
    {"repeat", (getter) Key_get_repeat, NULL, "triggered when a key is held down", NULL},
    {"hold", (getter) Key_get_hold, NULL, "a key is held down", NULL},
    {NULL}
};

PyTypeObject KeyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Key",
    .tp_doc = "the keyboard input handler",
    .tp_basicsize = sizeof(Key),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = (newfunc) Key_new,
    .tp_getattro = (getattrofunc) Key_getattro,
    .tp_getset = Key_getset
};