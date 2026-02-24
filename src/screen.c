#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "main.h"

static inline void filter(Filter *filter) {
    glUseProgram(filter -> src);
    shader.active = 0;
}

static inline void swap(Screen *self) {
    GLuint base = self -> a;

    self -> a = self -> b;
    self -> b = base;

    glBindTexture(GL_TEXTURE_2D, base);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self -> a, 0);
}

static inline void frame(Screen *self) {
    if (self != shader.screen) {
        glBindFramebuffer(GL_FRAMEBUFFER, (shader.screen = self) -> buffer);
        glViewport(0, 0, self -> base.size.x * window.ratio, self -> base.size.y * window.ratio);

        if (shader.active)
            glUniform2f(shader.active -> size, self -> base.size.x, -self -> base.size.y);
    }
}

static void draw(Screen *self) {
    base_matrix((Base *) self, &shader.image, self -> base.size.x, self -> base.size.y);

    glBindTexture(GL_TEXTURE_2D, self -> a);
    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static int size(Screen *self) {
    const double x = self -> base.size.x * window.ratio;
    const double y = self -> base.size.y * window.ratio;

    glBindTexture(GL_TEXTURE_2D, self -> a);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, self -> b);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    if (shader.screen == self) {
        glViewport(0, 0, x, y);

        if (shader.active)
            glUniform2f(shader.active -> size, self -> base.size.x, self -> base.size.y);
    }

    return 0;
}

// static Filter *filter(Screen *self) {
//     Filter *filter = self -> current ? self -> current -> next : self -> filters;

//     if (!filter) {
//         filter = malloc(sizeof(Filter));

//         if (!filter)
//             return PyErr_NoMemory(), NULL;

//         if (!self -> filters)
//             self -> filters = filter;

//         self -> current -> next = filter;
//     }

//     return self -> current = filter;
// }

static PyObject *screen_get_width(Screen *self, void *closure) {
    return PyFloat_FromDouble(self -> base.size.x);
}

static int screen_set_width(Screen *self, PyObject *value, void *closure) {
    DEL(value, "width")
    return ERR(self -> base.size.x = PyFloat_AsDouble(value)) ? -1 : size(self);
}

static PyObject *screen_get_height(Screen *self, void *closure) {
    return PyFloat_FromDouble(self -> base.size.y);
}

static int screen_set_height(Screen *self, PyObject *value, void *closure) {
    DEL(value, "height")
    return ERR(self -> base.size.y = PyFloat_AsDouble(value)) ? -1 : size(self);
}

static Vector *screen_get_size(Screen *self, void *closure) {
    Vector *vect = vector_new((PyObject *) self, (double *) &self -> base.size, 2, (int (*)(PyObject *)) size);

    if (vect) {
        vect -> names[x] = 'x';
        vect -> names[y] = 'y';
    }

    return vect;
}

static int screen_set_size(Screen *self, PyObject *value, void *closure) {
    DEL(value, "size")
    return vector_set(value, (double *) &self -> base.size, 2) ? -1 : size(self);
}

static Screen *screen_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Screen *self = (Screen *) type -> tp_alloc(type, 0);

    if (self) {
        // GLfloat border[4];
        GLuint textures[2];

        // glGenBuffers(1, &self -> uniform);
        // glBindBuffer(GL_UNIFORM_BUFFER, self -> uniform);
        // glBufferData(GL_UNIFORM_BUFFER, sizeof self -> filters, NULL, GL_DYNAMIC_DRAW);
        // glBindBufferBase(GL_UNIFORM_BUFFER, 1, self -> uniform);

        glGenFramebuffers(1, &self -> buffer);
        glGenTextures(2, textures);

        glBindTexture(GL_TEXTURE_2D, self -> a = textures[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, self -> b = textures[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        frame(self);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self -> a, 0);

        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glBindTexture(GL_TEXTURE_2D, 0);
    }

    return self;
}

static int screen_init(Screen *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"width", "height", NULL};

    base_data.type -> tp_init((PyObject *) self, NULL, NULL);
    self -> base.size.x = window.size.x;
    self -> base.size.y = window.size.y;

    self -> base.base.color.x = 1;
    self -> base.base.color.y = 1;
    self -> base.base.color.z = 1;

    return PyArg_ParseTupleAndKeywords(
        args, kwds, "|dd:Screen", kwlist,
        &self -> base.size.x,
        &self -> base.size.y) ? size(self) : -1;
}

static PyObject *screen_draw(Screen *self, PyObject *args) {
    unbind();
    draw(self);

    Py_RETURN_NONE;
}

static PyObject *screen_blit(Screen *self, PyObject *item) {
    return screen_bind((Base *) self, item, (void (*)(Base *)) draw);
}

static PyObject *screen_save(Screen *self, PyObject *item) {
    const char *path = PyUnicode_AsUTF8(item);

    if (!path)
        return NULL;

    const char *ext = strrchr(path, '.') + 1;
    const bool hdr = !strcmp(ext, "hdr");

    const double x = self -> base.size.x * window.ratio;
    const double y = self -> base.size.y * window.ratio;

    const size_t size = hdr ? sizeof(GLfloat) : sizeof(GLubyte);
    void *buffer = malloc(x * y * 4 * size);

    if (!buffer)
        return PyErr_NoMemory();

    frame(self);
    glReadPixels(0, 0, x, y, GL_RGBA, hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, buffer);

    int status = strcmp(ext, "bmp") ? strcmp(ext, "jpg") ? strcmp(ext, "tga") ? hdr ?
        stbi_write_hdr(path, x, y, 4, buffer) :
        stbi_write_png(path, x, y, 4, buffer, x * 4) :
        stbi_write_tga(path, x, y, 4, buffer) :
        stbi_write_jpg(path, x, y, 4, buffer, 80) :
        stbi_write_bmp(path, x, y, 4, buffer);

    free(buffer);
    return status ? Py_None : (PyErr_Format(PyExc_SystemError, "Failed to save image '%s'", path), NULL);
}

static PyObject *screen_warp(Screen *self, PyObject *args) {
    float radius = 100;
    float value = 2;

    if (!PyArg_ParseTuple(args, "|ff:warp", &radius, &value))
        return NULL;

    frame(self);
    swap(self);

    glBindVertexArray(shader.vao);
    filter(&shader.warp);

    glUniform2f(shader.warp.data, radius, value);
    glDisable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_BLEND);

    Py_RETURN_NONE;
}

// static PyObject *screen_blur(Screen *self, PyObject *args) {
//     // Filter *src = &self -> filters[self -> index];

//     // self -> index ++;
//     // src -> type = BLUR;
//     // src -> mode = 0;
//     // src -> a = 1;

//     // return PyArg_ParseTuple(args, "|fi:blur", &src -> a, &src -> mode) ? Py_None : NULL;
// }

static PyObject *screen_clear(Screen *self, PyObject *args) {
    frame(self);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(window.color.x, window.color.y, window.color.z, 1);

    Py_RETURN_NONE;
}

static void screen_dealloc(Screen *self) {
    GLuint textures[] = {self -> a, self -> b};

    glDeleteFramebuffers(1, &self -> buffer);
    glDeleteTextures(2, textures);
}

PyObject *screen_bind(Base *object, PyObject *item, void (*draw)(Base *)) {
    Screen *self;
    uint8_t mode = 0;

    if (!PyArg_ParseTuple(item, "O!|b:bind", screen_data.type, &self, &mode))
        return NULL;

    if (shader.mode != mode) {
        shader.mode = mode;

        if (mode == 1) {
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        }

        else glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    }

    frame(self);
    draw(object);

    Py_RETURN_NONE;
}

static PyGetSetDef screen_getset[] = {
    {"width", (getter) screen_get_width, (setter) screen_set_width, "The width of the surface", NULL},
    {"height", (getter) screen_get_height, (setter) screen_set_height, "The height of the surface", NULL},
    {"size", (getter) screen_get_size, (setter) screen_set_size, "The dimensions of the surface", NULL},
    {NULL}
};

static PyMethodDef screen_methods[] = {
    {"draw", (PyCFunction) screen_draw, METH_NOARGS, "Draw the offscreen texture on the main screen"},
    {"blit", (PyCFunction) screen_blit, METH_VARARGS, "Draw the offscreen texture to another offscreen surface"},
    {"save", (PyCFunction) screen_save, METH_O, "Save the offscreen texture to a file"},
    {"clear", (PyCFunction) screen_clear, METH_NOARGS, "Clear the screen to transparent pixels"},
    {"warp", (PyCFunction) screen_warp, METH_VARARGS, ""},
    // {"blur", (PyCFunction) screen_blur, METH_VARARGS, ""},
    {NULL}
};

static PyType_Slot screen_slots[] = {
    {Py_tp_doc, "A surface for drawing things offscreen"},
    {Py_tp_new, screen_new},
    {Py_tp_init, screen_init},
    {Py_tp_dealloc, screen_dealloc},
    {Py_tp_getset, screen_getset},
    {Py_tp_methods, screen_methods},
    {0}
};

Spec screen_data = {{"Screen", sizeof(Screen), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, screen_slots}};