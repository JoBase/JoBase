#include <main.h>

#ifdef _WIN32
PyMODINIT_FUNC PyInit___init__;
#endif

static PyObject *loop;
FT_Library library;

Texture *textures;
Font *fonts;

GLint uniforms[5];
GLuint program;
GLuint mesh;

PyObject *module;
Window *window;
Cursor *cursor;
Camera *camera;
Key *key;

static void cleanup() {
    while (textures) {
        Texture *this = textures;

        textures = this -> next;
        glDeleteTextures(1, &this -> src);

        free(this -> name);
        free(this);
    }

    while (fonts) {
        Font *this = fonts;

        fonts = this -> next;
        FT_Done_Face(this -> face);

        free(this -> name);
        free(this);
    }

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &mesh);

    FT_Done_FreeType(library);
    glfwTerminate();
}

static int update() {
    const double sx = 2 / window -> size.x * camera -> scale.x;
    const double sy = 2 / window -> size.y * camera -> scale.y;

    GLfloat matrix[] = {
        sx, 0, 0, 0, sy, 0,
        -camera -> pos.x * sx, -camera -> pos.y * sy, -1
    };
    
    glUniformMatrix3fv(uniforms[view], 1, GL_FALSE, matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    if (PyErr_CheckSignals() || (loop && !PyObject_CallObject(loop, NULL)))
        return -1;

    window -> resize = false;
    cursor -> move = false;
    cursor -> enter = false;
    cursor -> leave = false;
    cursor -> press = false;
    cursor -> release = false;
    key -> press = false;
    key -> release = false;
    key -> repeat = false;

    for (uint8_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; i ++) {
        cursor -> buttons[i].press = false;
        cursor -> buttons[i].release = false;
    }

    for (uint16_t i = 0; i < GLFW_KEY_LAST; i ++) {
        key -> keys[i].press = false;
        key -> keys[i].release = false;
        key -> keys[i].repeat = false;
    }

    glfwSwapBuffers(window -> glfw);
    return glfwPollEvents(), 0;
}

#ifdef __EMSCRIPTEN__
static void run() {
    if (update())
        emscripten_cancel_main_loop();
}
#endif

static const char *filepath(char *path, size_t length, const char *file) {
    return path[length] = 0, strcat(path, file);
}

static void error_callback(int code, const char *description) {
    fprintf(stderr, "%s\n", description);
}

static PyObject *Module_random(PyObject *self, PyObject *args) {
    double x = 0, y = 1;

    if (PyArg_ParseTuple(args, "|dd:random", &x, &y))
        return PyFloat_FromDouble(rand() / (RAND_MAX / fabs(y - x)) + MIN(x, y));

    return NULL;
}

static PyObject *Module_randint(PyObject *self, PyObject *args) {
    int x = 0, y = 1;

    if (PyArg_ParseTuple(args, "|ii:randint", &x, &y))
        return PyLong_FromLong(rand() / (RAND_MAX / abs(y - x + 1)) + MIN(x, y));

    return NULL;
}

static PyObject *Module_hypot(PyObject *self, PyObject *args) {
    double x, y;

    if (PyArg_ParseTuple(args, "dd:hypot", &x, &y))
        return PyFloat_FromDouble(hypot(x, y));

    return NULL;
}

static PyObject *Module_sin(PyObject *self, PyObject *value) {
    const double angle = PyFloat_AsDouble(value);
    return ERR(angle) ? NULL : PyFloat_FromDouble(sin(angle));
}

static PyObject *Module_cos(PyObject *self, PyObject *value) {
    const double angle = PyFloat_AsDouble(value);
    return ERR(angle) ? NULL : PyFloat_FromDouble(cos(angle));
}

static PyObject *Module_tan(PyObject *self, PyObject *value) {
    const double angle = PyFloat_AsDouble(value);
    return ERR(angle) ? NULL : PyFloat_FromDouble(tan(angle));
}

static PyObject *Module_asin(PyObject *self, PyObject *value) {
    const double angle = PyFloat_AsDouble(value);
    return ERR(angle) ? NULL : PyFloat_FromDouble(asin(angle));
}

static PyObject *Module_acos(PyObject *self, PyObject *value) {
    const double angle = PyFloat_AsDouble(value);
    return ERR(angle) ? NULL : PyFloat_FromDouble(acos(angle));
}

static PyObject *Module_atan(PyObject *self, PyObject *value) {
    const double angle = PyFloat_AsDouble(value);
    return ERR(angle) ? NULL : PyFloat_FromDouble(atan(angle));
}

static PyObject *Module_sqrt(PyObject *self, PyObject *value) {
    const double number = PyFloat_AsDouble(value);
    return ERR(number) ? NULL : PyFloat_FromDouble(sqrt(number));
}

static PyObject *Module_cbrt(PyObject *self, PyObject *value) {
    const double number = PyFloat_AsDouble(value);
    return ERR(number) ? NULL : PyFloat_FromDouble(cbrt(number));
}

static PyObject *Module_ceil(PyObject *self, PyObject *value) {
    const double number = PyFloat_AsDouble(value);
    return ERR(number) ? NULL : PyFloat_FromDouble(ceil(number));
}

static PyObject *Module_floor(PyObject *self, PyObject *value) {
    const double number = PyFloat_AsDouble(value);
    return ERR(number) ? NULL : PyFloat_FromDouble(floor(number));
}

static PyObject *Module_run(PyObject *self, PyObject *ignored) {
    glfwShowWindow(window -> glfw);

    if (PyObject_HasAttrString(module, "loop") && !(loop = PyObject_GetAttrString(module, "loop")))
        return NULL;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(run, 0, true);
#endif

    while (!glfwWindowShouldClose(window -> glfw))
        if (update()) return NULL;

    Py_RETURN_NONE;
}

static PyMethodDef Module_methods[] = {
    {"run", Module_run, METH_NOARGS, "run the main game loop"},
    {"random", Module_random, METH_VARARGS, "find a random number between two numbers"},
    {"randint", Module_randint, METH_VARARGS, "find a random integer between two integers"},
    {"hypot", Module_hypot, METH_VARARGS, "find the root of the square of x and y"},
    {"sin", Module_sin, METH_O, "sine function of an angle in radians"},
    {"cos", Module_cos, METH_O, "cosine function of an angle in radians"},
    {"tan", Module_tan, METH_O, "tangent function of an angle in radians"},
    {"asin", Module_asin, METH_O, "arcsine function of an angle in radians"},
    {"acos", Module_acos, METH_O, "arccosine function of an angle in radians"},
    {"atan", Module_atan, METH_O, "arctangent function of an angle in radians"},
    {"sqrt", Module_sqrt, METH_O, "find the square root"},
    {"cbrt", Module_cbrt, METH_O, "find the cube root"},
    {"ceil", Module_ceil, METH_O, "round up to the nearest whole number"},
    {"floor", Module_floor, METH_O, "round down to the nearest whole number"},
    {NULL}
};

static int Module_exec(PyObject *self) {
    #define COLOR(r, g, b) PyTuple_Pack(3,PyFloat_FromDouble(r),PyFloat_FromDouble(g),PyFloat_FromDouble(b))
    #define ADD(e, t) Py_INCREF(e);if(PyModule_AddObject(module,#t,(PyObject*)t)){Py_XDECREF(t);return -1;}
    #define PATH(e) (path[length]=0,strcat(path,e));

    PyObject *modules = PySys_GetObject("modules");
    PyObject *file = PyObject_GetAttrString(self, "__file__");
    INIT(!file)

    Py_ssize_t size;
    const char *string = PyUnicode_AsUTF8AndSize(file, &size);

    glfwSetErrorCallback(error_callback);
    Py_DECREF(file);
    INIT(!string)

    const char *last = strrchr(string, '/');
    const size_t length = size - strlen(last ? last : strrchr(string, '\\')) + 1;
    char *path = strdup(string);

    if (!glfwInit()) {
        PyErr_SetString(PyExc_OSError, "failed to initialize GLFW");
        return -1;
    }

    if (FT_Init_FreeType(&library)) {
        PyErr_SetString(PyExc_OSError, "failed to initialize FreeType");
        return glfwTerminate(), -1;
    }

    Py_INCREF(module = PyDict_GetItemString(modules, "__main__"));
    Py_AtExit(cleanup);

    ADD(window = (Window *) PyObject_CallObject((PyObject *) &WindowType, NULL), window)
    ADD(cursor = (Cursor *) PyObject_CallObject((PyObject *) &CursorType, NULL), cursor)
    ADD(camera = (Camera *) PyObject_CallObject((PyObject *) &CameraType, NULL), camera)
    ADD(key = (Key *) PyObject_CallObject((PyObject *) &KeyType, NULL), key)

    const char *vs =
        "#version " VERSION "\n"

        "in vec2 vert;"
        "in vec2 coord;"
        "out vec2 pos;"

        "uniform mat3 view;"
        "uniform mat3 obj;"

        "void main() {"
            "gl_Position = vec4(view * obj * vec3(vert, 1), 1);"
            "pos = coord;"
        "}";

    const char *fs =
        "#version " VERSION "\n"
        "precision mediump float;"

        "in vec2 pos;"
        "out vec4 frag;"

        "uniform vec4 color;"
        "uniform sampler2D sampler;"
        "uniform int img;"

        "void main() {"
            "frag = color * (img == 0 ? vec4(1) : img == 1 ? texture(sampler, pos) : vec4(vec3(1), texture(sampler, pos).r));"
        "}";

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex, 1, &vs, NULL);
    glShaderSource(fragment, 1, &fs, NULL);
    glCompileShader(vertex);
    glCompileShader(fragment);

    glAttachShader(program = glCreateProgram(), vertex);
    glAttachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glLinkProgram(program);
    glUseProgram(program);

    GLuint buffer;
    GLfloat data[] = {-.5, .5, 0, 0, .5, .5, 1, 0, -.5, -.5, 0, 1, .5, -.5, 1, 1};
    GLint coord = glGetAttribLocation(program, "coord");

    uniforms[vert] = glGetAttribLocation(program, "vert");
    uniforms[view] = glGetUniformLocation(program, "view");
    uniforms[obj] = glGetUniformLocation(program, "obj");
    uniforms[color] = glGetUniformLocation(program, "color");
    uniforms[img] = glGetUniformLocation(program, "img");

    glGenVertexArrays(1, &mesh);
    glBindVertexArray(mesh);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);

    glVertexAttribPointer(uniforms[vert], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *) (sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(uniforms[vert]);
    glEnableVertexAttribArray(coord);

    glBindVertexArray(0);
    glDeleteBuffers(1, &buffer);

    glEnable(GL_BLEND);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    INIT(PyModule_AddObject(module, "Rectangle", (PyObject *) &RectangleType))
    INIT(PyModule_AddObject(module, "Image", (PyObject *) &ImageType))
    INIT(PyModule_AddObject(module, "Text", (PyObject *) &TextType))
    INIT(PyModule_AddObject(module, "Circle", (PyObject *) &CircleType))
    INIT(PyModule_AddObject(module, "Shape", (PyObject *) &ShapeType))
    INIT(PyModule_AddObject(module, "Line", (PyObject *) &LineType))
    INIT(PyModule_AddObject(module, "Physics", (PyObject *) &PhysicsType))
    INIT(PyModule_AddObject(module, "Pin", (PyObject *) &PinType))
    INIT(PyModule_AddObject(module, "Spring", (PyObject *) &SpringType))
    INIT(PyModule_AddObject(module, "Groove", (PyObject *) &GrooveType))
    // INIT(PyModule_AddObject(module, "Pivot", (PyObject *) &PivotType))

    INIT(PyModule_AddIntConstant(module, "DYNAMIC", CP_BODY_TYPE_DYNAMIC))
    INIT(PyModule_AddIntConstant(module, "STATIC", CP_BODY_TYPE_KINEMATIC))

    INIT(PyModule_AddObject(module, "WHITE", COLOR(1, 1, 1)))
    INIT(PyModule_AddObject(module, "BLACK", COLOR(0, 0, 0)))
    INIT(PyModule_AddObject(module, "GRAY", COLOR(.5, .5, .5)))
    INIT(PyModule_AddObject(module, "DARK_GRAY", COLOR(.2, .2, .2)))
    INIT(PyModule_AddObject(module, "LIGHT_GRAY", COLOR(.8, .8, .8)))
    INIT(PyModule_AddObject(module, "BROWN", COLOR(.6, .2, .2)))
    INIT(PyModule_AddObject(module, "TAN", COLOR(.8, .7, .6)))
    INIT(PyModule_AddObject(module, "RED", COLOR(1, 0, 0)))
    INIT(PyModule_AddObject(module, "DARK_RED", COLOR(.6, 0, 0)))
    INIT(PyModule_AddObject(module, "SALMON", COLOR(1, .5, .5)))
    INIT(PyModule_AddObject(module, "ORANGE", COLOR(1, .5, 0)))
    INIT(PyModule_AddObject(module, "GOLD", COLOR(1, .8, 0)))
    INIT(PyModule_AddObject(module, "YELLOW", COLOR(1, 1, 0)))
    INIT(PyModule_AddObject(module, "OLIVE", COLOR(.5, .5, 0)))
    INIT(PyModule_AddObject(module, "LIME", COLOR(0, 1, 0)))
    INIT(PyModule_AddObject(module, "DARK_GREEN", COLOR(0, .4, 0)))
    INIT(PyModule_AddObject(module, "GREEN", COLOR(0, .5, 0)))
    INIT(PyModule_AddObject(module, "AQUA", COLOR(0, 1, 1)))
    INIT(PyModule_AddObject(module, "BLUE", COLOR(0, 0, 1)))
    INIT(PyModule_AddObject(module, "LIGHT_BLUE", COLOR(.5, .8, 1)))
    INIT(PyModule_AddObject(module, "AZURE", COLOR(.9, 1, 1)))
    INIT(PyModule_AddObject(module, "NAVY", COLOR(0, 0, .5)))
    INIT(PyModule_AddObject(module, "PURPLE", COLOR(.5, 0, 1)))
    INIT(PyModule_AddObject(module, "PINK", COLOR(1, .75, .8)))
    INIT(PyModule_AddObject(module, "MAGENTA", COLOR(1, 0, 1)))
    INIT(PyModule_AddObject(module, "PI", PyFloat_FromDouble(M_PI)))

    INIT(PyModule_AddStringConstant(module, "MAN", filepath(path, length, "images/man.png")))
    INIT(PyModule_AddStringConstant(module, "COIN", filepath(path, length, "images/coin.png")))
    INIT(PyModule_AddStringConstant(module, "ENEMY", filepath(path, length, "images/enemy.png")))
    INIT(PyModule_AddStringConstant(module, "DEFAULT", filepath(path, length, "fonts/default.ttf")))
    INIT(PyModule_AddStringConstant(module, "CODE", filepath(path, length, "fonts/code.ttf")))
    INIT(PyModule_AddStringConstant(module, "PENCIL", filepath(path, length, "fonts/pencil.ttf")))
    INIT(PyModule_AddStringConstant(module, "SERIF", filepath(path, length, "fonts/serif.ttf")))
    INIT(PyModule_AddStringConstant(module, "HANDWRITING", filepath(path, length, "fonts/handwriting.ttf")))
    INIT(PyModule_AddStringConstant(module, "TYPEWRITER", filepath(path, length, "fonts/typewriter.ttf")))
    INIT(PyModule_AddStringConstant(module, "JOINED", filepath(path, length, "fonts/joined.ttf")))

    return free(path), PyModule_AddFunctions(module, Module_methods);
}

static int Module_traverse(PyObject *self, visitproc visit, void *arg) {
    Py_VISIT(window);
    Py_VISIT(cursor);
    Py_VISIT(camera);
    Py_VISIT(key);

    return 0;
}

static int Module_clear(PyObject *self) {
    Py_CLEAR(window);
    Py_CLEAR(cursor);
    Py_CLEAR(camera);
    Py_CLEAR(key);

    return 0;
}

static void Module_free(void *closure) {
    Py_CLEAR(window);
    Py_CLEAR(cursor);
    Py_CLEAR(camera);
    Py_CLEAR(key);

    Py_DECREF(module);
    Py_XDECREF(loop);
}

static PyModuleDef_Slot Module_slots[] = {
    {Py_mod_exec, Module_exec},
    {0, NULL}
};

static PyModuleDef Module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "JoBase",
    .m_size = 0,
    .m_slots = Module_slots,
    .m_traverse = Module_traverse,
    .m_clear = Module_clear,
    .m_free = Module_free
};

PyMODINIT_FUNC PyInit_JoBase() {
    #define READY(e) if(PyType_Ready(&e))return NULL;

    printf("Welcome to JoBase\n");
    srand(time(NULL));

    READY(PointsType)
    READY(VectorType)
    READY(ButtonType)
    READY(WindowType)
    READY(CursorType)
    READY(CameraType)
    READY(KeyType)
    READY(BaseType)
    READY(RectangleType)
    READY(ImageType)
    READY(TextType)
    READY(CircleType)
    READY(ShapeType)
    READY(LineType)
    READY(PhysicsType)
    READY(GroupType)
    READY(BodyType)
    READY(JointType)
    READY(PinType)
    READY(SpringType)
    READY(GrooveType)
    // READY(PivotType)

    return PyModuleDef_Init(&Module);
}