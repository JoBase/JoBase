#define STB_IMAGE_IMPLEMENTATION

#include <main.h>
#include <stb_image/stb_image.h>

static int load(Image *self, const char *name) {
    for (Texture *this = textures; this; this = this -> next)
        if (!strcmp(this -> name, name))
            return self -> src = this, 0;

    int width, height;
    stbi_uc *image = stbi_load(name, &width, &height, 0, STBI_rgb_alpha);

    if (!image) {
        PyErr_Format(PyExc_FileNotFoundError, "failed to load image: \"%s\"", name);
        return -1;
    }

    NEW(Texture, textures)
    glGenTextures(1, &textures -> src);
    glBindTexture(GL_TEXTURE_2D, textures -> src);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    textures -> size.x = width;
    textures -> size.y = height;

    textures -> name = strdup(name);
    self -> src = textures;

    return stbi_image_free(image), 0;
}

static PyObject *Image_get_name(Image *self, void *closure) {
    return PyUnicode_FromString(self -> src -> name);
}

static int Image_set_name(Image *self, PyObject *value, void *closure) {
    DEL(value, "name")

    const char *name = PyUnicode_AsUTF8(value);
    INIT(!name || load(self, name))

    self -> base.size.x = self -> src -> size.x;
    self -> base.size.y = self -> src -> size.y;

    return 0;
}

static PyObject *Image_draw(Image *self, PyObject *args) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self -> src -> src);

    Rectangle_render(&self -> base, image);
    glBindTexture(GL_TEXTURE_2D, 0);

    Py_RETURN_NONE;
}

static int Image_init(Image *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", "x", "y", "angle", "width", "height", "color", NULL};

    BaseType.tp_init((PyObject *) self, NULL, NULL);
    self -> base.size.x = 0;
    self -> base.size.y = 0;

    self -> base.base.color.r = 1;
    self -> base.base.color.g = 1;
    self -> base.base.color.b = 1;
    
    PyObject *color = NULL;
    PyObject *src = PyObject_GetAttrString(module, "MAN");
    INIT(!src)

    const char *name = PyUnicode_AsUTF8(src);
    Py_DECREF(src);

    INIT(!name || !PyArg_ParseTupleAndKeywords(
        args, kwds, "|sdddddO:Image", kwlist, &name, &self -> base.base.pos.x,
        &self -> base.base.pos.y, &self -> base.base.angle, &self -> base.size.x,
        &self -> base.size.y, &color) || load(self, name))

    self -> base.size.x = self -> base.size.x ? self -> base.size.x : self -> src -> size.x;
    self -> base.size.y = self -> base.size.y ? self -> base.size.y : self -> src -> size.y;

    return Vector_set(color, (vec) &self -> base.base.color, 4);
}

static PyGetSetDef Image_getset[] = {
    {"name", (getter) Image_get_name, (setter) Image_set_name, "path to the image source", NULL},
    {NULL}
};

static PyMethodDef Image_methods[] = {
    {"draw", (PyCFunction) Image_draw, METH_NOARGS, "draw the image on the screen"},
    {NULL}
};

PyTypeObject ImageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Image",
    .tp_doc = "draw images on the screen",
    .tp_basicsize = sizeof(Image),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &RectangleType,
    .tp_new = (newfunc) Rectangle_new,
    .tp_init = (initproc) Image_init,
    .tp_methods = Image_methods,
    .tp_getset = Image_getset
};