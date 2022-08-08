#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <main.h>
#include <stb_image/stb_image.h>

static PyObject *Image_draw(Image *self, PyObject *Py_UNUSED(ignored)) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self -> texture -> src);

    rectangleDraw(&self -> rect, IMAGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    Py_RETURN_NONE;
}

static PyObject *Image_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return rectangleNew(type);
}

static int Image_init(Image *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", "x", "y", "angle", "width", "height", NULL};
    double sx = 0, sy = 0;

    const char *name = filepath("images/man.png");
    baseInit((Base *) self);

    if (!PyArg_ParseTupleAndKeywords(
        args, kwds, "|sdddddO", kwlist, &name, &self -> rect.base.pos[x],
        &self -> rect.base.pos[y], &self -> rect.base.angle,
        &sx, &sy)) return -1;

    self -> rect.base.color[r] = 1;
    self -> rect.base.color[g] = 1;
    self -> rect.base.color[b] = 1;

    for (Texture *this = textures; this; this = this -> next)
        if (!strcmp(this -> name, name)) {
            self -> texture = this;
            self -> rect.size[x] = sx ? sx : this -> size.x;
            self -> rect.size[y] = sy ? sy : this -> size.y;
            return 0;
        }

    int width, height;
    stbi_uc *image = stbi_load(name, &width, &height, 0, STBI_rgb_alpha);
    if (!image) return format(PyExc_FileNotFoundError, "failed to load image: \"%s\"", name), -1;

    NEW(Texture, textures)
    glGenTextures(1, &textures -> src);
    glBindTexture(GL_TEXTURE_2D, textures -> src);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    parameters();
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    self -> texture = textures;
    self -> rect.size[x] = sx ? sx : width;
    self -> rect.size[y] = sy ? sy : height;
    textures -> size.x = width;
    textures -> size.y = height;
    textures -> name = strdup(name);

    return 0;
}

static PyMethodDef ImageMethods[] = {
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
    .tp_new = Image_new,
    .tp_init = (initproc) Image_init,
    .tp_methods = ImageMethods
};