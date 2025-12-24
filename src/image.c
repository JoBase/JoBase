#define STB_IMAGE_IMPLEMENTATION
#include "main.h"

static void draw(Image *self) {
    glBindTexture(GL_TEXTURE_2D, self -> src -> src);
    glUseProgram(shader.image.src);
    base_matrix(&self -> base.base, shader.image.obj, shader.image.color, self -> base.size.x, self -> base.size.y);

    glBindVertexArray(shader.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static int load(Image *self, const char *name) {
    for (Texture *this = textures; this; this = this -> next)
        if (!strcmp(this -> name, name))
            return self -> src = this, 0;

    Texture *texture = malloc(sizeof(Texture));

    if (texture) {
        if ((texture -> name = strdup(name))) {
            texture -> next = textures;
            textures = self -> src = texture;

            int width, height;
            stbi_uc *image = stbi_load(name, &width, &height, 0, STBI_rgb_alpha);

            if (!image)
                return PyErr_Format(PyExc_FileNotFoundError, "Failed to load image '%s', %s", name, stbi_failure_reason()), -1;

            glGenTextures(1, &texture -> src);
            glBindTexture(GL_TEXTURE_2D, texture -> src);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            texture -> width = width;
            texture -> height = height;

            return 0;
        }

        free(texture);
    }

    return PyErr_NoMemory(), -1;
}

static PyObject *image_get_name(Image *self, void *closure) {
    return PyUnicode_FromString(self -> src -> name);
}

static int image_set_name(Image *self, PyObject *value, void *closure) {
    DEL(value, "name")

    const char *name = PyUnicode_AsUTF8(value);
    INIT(!name || load(self, name))

    self -> base.size.x = self -> src -> width;
    self -> base.size.y = self -> src -> height;

    return 0;
}

static int image_init(Image *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", "x", "y", "angle", "width", "height", "color", NULL};

    PyObject *color = NULL;
    const char *name = NULL;

    base_data.type -> tp_init((PyObject *) self, NULL, NULL);
    self -> base.size.x = 0;
    self -> base.size.y = 0;

    self -> base.base.color.x = 1;
    self -> base.base.color.y = 1;
    self -> base.base.color.z = 1;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sdddddO:Image", kwlist,
        &name, &self -> base.base.pos.x,
        &self -> base.base.pos.y,
        &self -> base.base.angle,
        &self -> base.size.x,
        &self -> base.size.y, &color));

    if (!name) {
        sprintf(path.src + path.size, "images/man.png");
        name = path.src;
    }

    INIT(load(self, name))

    self -> base.size.x = self -> base.size.x ? self -> base.size.x : self -> src -> width;
    self -> base.size.y = self -> base.size.y ? self -> base.size.y : self -> src -> height;

    return vector_set(color, (double *) &self -> base.base.color, 4);
}

static PyObject *image_draw(Image *self, PyObject *args) {
    draw(self);
    Py_RETURN_NONE;
}

static PyObject *image_blit(Image *self, PyObject *item) {
    if (screen_bind(item))
        return NULL;

    draw(self);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Py_RETURN_NONE;
}

static PyGetSetDef image_getset[] = {
    {"name", (getter) image_get_name, (setter) image_set_name, "The filepath to the source image", NULL},
    {NULL}
};

static PyMethodDef image_methods[] = {
    {"draw", (PyCFunction) image_draw, METH_NOARGS, "Draw the image on the screen"},
    {"blit", (PyCFunction) image_blit, METH_O, "Render the image to an offscreen surface"},
    {NULL}
};

static PyType_Slot image_slots[] = {
    {Py_tp_doc, "Render images on the screen"},
    {Py_tp_new, PyType_GenericNew},
    {Py_tp_init, image_init},
    {Py_tp_getset, image_getset},
    {Py_tp_methods, image_methods},
    {0}
};

Spec image_data = {{"Image", sizeof(Image), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, image_slots}};