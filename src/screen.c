#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "main.h"

static void draw(Screen *self) {
    glUseProgram(shader.image.src);
    base_matrix(&self -> base.base, shader.image.obj, shader.image.color, self -> base.size.x, self -> base.size.y);

    glBindTexture(GL_TEXTURE_2D, self -> texture);
    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static int size(Screen *self) {
    glBindTexture(GL_TEXTURE_2D, self -> texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self -> base.size.x * window.ratio, self -> base.size.y * window.ratio, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // glBindFramebuffer(GL_FRAMEBUFFER, self -> buffer);
    // glClearColor(0, 0, 0, 0);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glClearColor(window.color.x, window.color.y, window.color.z, 1);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return glBindTexture(GL_TEXTURE_2D, 0), 0;
}

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
        glGenFramebuffers(1, &self -> buffer);
        glGenTextures(1, &self -> texture);

        glBindTexture(GL_TEXTURE_2D, self -> texture);
        glBindFramebuffer(GL_FRAMEBUFFER, self -> buffer);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self -> texture, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
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
    draw(self);
    Py_RETURN_NONE;
}

static PyObject *screen_blit(Screen *self, PyObject *item) {
    if (screen_bind(item))
        return NULL;

    draw(self);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Py_RETURN_NONE;
}

static PyObject *screen_save(Screen *self, PyObject *item) {
    const char *path = PyUnicode_AsUTF8(item);

    if (!path)
        return NULL;

    const char *ext = strrchr(path, '.') + 1;
    const bool hdr = !strcmp(ext, "hdr");

    const size_t size = hdr ? sizeof(GLfloat) : sizeof(GLubyte);
    void *buffer = malloc(self -> base.size.x * self -> base.size.y * 4 * size * window.ratio);

    if (!buffer)
        return PyErr_NoMemory();

    glBindFramebuffer(GL_FRAMEBUFFER, self -> buffer);
    glReadPixels(0, 0, self -> base.size.x, self -> base.size.y, GL_RGBA, hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, buffer);

    int status = strcmp(ext, "bmp") ? strcmp(ext, "jpg") ? strcmp(ext, "tga") ? hdr ?
        stbi_write_hdr(path, self -> base.size.x, self -> base.size.y, 4, buffer) :
        stbi_write_png(path, self -> base.size.x, self -> base.size.y, 4, buffer, self -> base.size.x * 4) :
        stbi_write_tga(path, self -> base.size.x, self -> base.size.y, 4, buffer) :
        stbi_write_jpg(path, self -> base.size.x, self -> base.size.y, 4, buffer, 80) :
        stbi_write_bmp(path, self -> base.size.x, self -> base.size.y, 4, buffer);

    free(buffer);
    return status ? Py_None : (PyErr_Format(PyExc_SystemError, "Failed to save image '%s', %s", path, stbi_failure_reason()), NULL);
}

static void screen_dealloc(Screen *self) {
    glDeleteFramebuffers(1, &self -> buffer);
    glDeleteTextures(1, &self -> texture);
}

int screen_bind(PyObject *item) {
    if (!PyObject_TypeCheck(item, screen_data.type))
        return PyErr_SetString(PyExc_TypeError, "Parameter passed to blit() must be a Screen"), -1;

    return glBindFramebuffer(GL_FRAMEBUFFER, ((Screen *) item) -> buffer), 0;
}

static PyGetSetDef screen_getset[] = {
    {"width", (getter) screen_get_width, (setter) screen_set_width, "The width of the surface", NULL},
    {"height", (getter) screen_get_height, (setter) screen_set_height, "The height of the surface", NULL},
    {"size", (getter) screen_get_size, (setter) screen_set_size, "The dimensions of the surface", NULL},
    {NULL}
};

static PyMethodDef screen_methods[] = {
    {"draw", (PyCFunction) screen_draw, METH_NOARGS, "Draw the offscreen texture on the main screen"},
    {"blit", (PyCFunction) screen_blit, METH_O, "Draw the offscreen texture to another offscreen surface"},
    {"save", (PyCFunction) screen_save, METH_O, "Save the offscreen texture to a file"},
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