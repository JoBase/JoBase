#include "main.h"

static void draw(Text *self) {
    use(&shader.text);

    GLfloat matrix[] = {
        self -> size * self -> base.scale.x, 0, 0,
        0, self -> size * self -> base.scale.y, 0,
        self -> base.pos.x - self -> width * self -> size / 2,
        self -> base.pos.y, 1
    };

    glBindVertexArray(self -> vao);
    glBindTexture(GL_TEXTURE_2D, self -> src -> src);

    glUniformMatrix3fv(shader.text.obj, 1, GL_FALSE, matrix);
    glUniform4f(shader.text.color, self -> base.color.x, self -> base.color.y, self -> base.color.z, self -> base.color.w);
    glDrawArrays(GL_TRIANGLES, 0, self -> len * 6);
}

static int compare(wchar_t *code, Glyph *glyph) {
    return *code - glyph -> code;
}

static int create(Text *self) {
    double advance = 0;

    const size_t size = self -> len * 24 * sizeof(GLfloat);
    GLfloat *data = malloc(size);

    if (!data)
        return PyErr_NoMemory(), -1;

    for (size_t i = 0; i < self -> len; i ++) {
        size_t j = i * 24;
        wchar_t item = self -> content[i];

        Glyph *glyph = (unsigned) item < self -> src -> len ?
            &self -> src -> chars[self -> src -> chars[item].jump] :
            bsearch(&item, self -> src -> chars, self -> src -> len, sizeof(Glyph), (int (*)(const void *, const void *)) compare);

        if (!glyph)
            glyph = self -> src -> chars;

        data[j] = data[j + 12] = data[j + 20] = advance + glyph -> x0;
        data[j + 1] = data[j + 5] = data[j + 13] = glyph -> y1;
        data[j + 2] = data[j + 14] = data[j + 22] = glyph -> s0;
        data[j + 3] = data[j + 7] = data[j + 15] = glyph -> t1;

        data[j + 4] = data[j + 8] = data[j + 16] = advance + glyph -> x1;
        data[j + 9] = data[j + 17] = data[j + 21] = glyph -> y0;
        data[j + 6] = data[j + 10] = data[j + 18] = glyph -> s1;
        data[j + 11] = data[j + 19] = data[j + 23] = glyph -> t0;

        advance += glyph -> adv;
    }

    self -> width = advance;

    glBindVertexArray(self -> vao);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    return free(data), 0;
}

static int content(Text *self, PyObject *value) {
    Py_ssize_t len = PyUnicode_AsWideChar(value, NULL, 0);
    INIT(len < 0)

    self -> len = len - 1;
    self -> content = realloc(self -> content, len * sizeof(wchar_t));

    return self -> content ? PyUnicode_AsWideChar(value, self -> content, self -> len) < 0 ? -1 : 0 : (PyErr_NoMemory(), -1);
}

static int load(Text *self, uint8_t src) {
    for (Font *this = fonts; this; this = this -> next)
        if (this -> id == src)
            return self -> src = this, 0;

    sprintf(path.src + path.size, "fonts/%02d.png", src);

    int width, height;
    stbi_uc *image = stbi_load(path.src, &width, &height, 0, STBI_rgb);

    if (image) {
        strcpy(path.src + path.size + 8, ".bin");
        FILE *file = fopen(path.src, "rb");

        if (file) {
            if (fseek(file, 0, SEEK_END))
                PyErr_SetFromErrno(PyExc_OSError);

            else {
                const long size = ftell(file);

                if (size < 0 || fseek(file, 0, SEEK_SET))
                    PyErr_SetFromErrno(PyExc_OSError);

                else {
                    Font *font = malloc(sizeof(Font));

                    if (font) {
                        if ((font -> chars = malloc(size))) {
                            font -> len = size / sizeof(Glyph);
                            font -> next = fonts;
                            font -> id = src;
                            fonts = self -> src = font;

                            if (fread(font -> chars, 1, size, file) == (unsigned) size) {
                                fclose(file);

                                glGenTextures(1, &font -> src);
                                glBindTexture(GL_TEXTURE_2D, font -> src);

                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                                return stbi_image_free(image), 0;
                            }

                            else PyErr_SetFromErrno(PyExc_OSError);
                        }

                        else {
                            PyErr_NoMemory();
                            free(font);
                        }
                    }

                    else PyErr_NoMemory();
                }
            }

            fclose(file);
        }

        else PyErr_SetFromErrno(PyExc_OSError);
        stbi_image_free(image);
    }

    else PyErr_Format(PyExc_FileNotFoundError, "Failed to load font, %s", stbi_failure_reason());
    return -1;
}

static PyObject *text_get_top(Text *self, void *closure) {
    return PyFloat_FromDouble(rect_y((Base *) self, self -> width * self -> size, self -> size, 1));
}

static int text_set_top(Text *self, PyObject *value, void *closure) {
    return base_top((Base *) self, value, rect_y((Base *) self, self -> width * self -> size, self -> size, 1));
}

static PyObject *text_get_right(Text *self, void *closure) {
    return PyFloat_FromDouble(rect_x((Base *) self, self -> width * self -> size, self -> size, 1));
}

static int text_set_right(Text *self, PyObject *value, void *closure) {
    return base_right((Base *) self, value, rect_x((Base *) self, self -> width * self -> size, self -> size, 1));
}

static PyObject *text_get_bottom(Text *self, void *closure) {
    return PyFloat_FromDouble(rect_y((Base *) self, self -> width * self -> size, self -> size, -1));
}

static int text_set_bottom(Text *self, PyObject *value, void *closure) {
    return base_bottom((Base *) self, value, rect_y((Base *) self, self -> width * self -> size, self -> size, -1));
}

static PyObject *text_get_left(Text *self, void *closure) {
    return PyFloat_FromDouble(rect_x((Base *) self, self -> width * self -> size, self -> size, -1));
}

static int text_set_left(Text *self, PyObject *value, void *closure) {
    return base_left((Base *) self, value, rect_x((Base *) self, self -> width * self -> size, self -> size, -1));
}

static PyObject *text_get_width(Text *self, void *closure) {
    return PyFloat_FromDouble(self -> width * self -> size);
}

static PyObject *text_get_size(Text *self, void *closure) {
    return PyFloat_FromDouble(self -> size);
}

static int text_set_size(Text *self, PyObject *value, void *closure) {
    DEL(value, "size")
    return ERR(self -> size = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *text_get_content(Text *self, void *closure) {
    return PyUnicode_FromWideChar(self -> content, self -> len);
}

static int text_set_content(Text *self, PyObject *value, void *closure) {
    DEL(value, "content")
    return content(self, value) ? -1 : create(self);
}

static PyObject *text_get_font(Text *self, void *closure) {
    return PyLong_FromUnsignedLong(self -> src -> id);
}

static int text_set_font(Text *self, PyObject *value, void *closure) {
    DEL(value, "font")

    const int src = PyLong_AsInt(value);
    return ERR(src) || load(self, src) ? -1 : create(self);
}

static Text *text_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Text *self = (Text *) type -> tp_alloc(type, 0);

    if (self) {
        glGenVertexArrays(1, &self -> vao);
        glGenBuffers(1, &self -> vbo);

        glBindVertexArray(self -> vao);
        glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *) (sizeof(GLfloat) * 2));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    return self;
}

static int text_init(Text *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"content", "x", "y", "size", "angle", "color", "font", NULL};

    PyObject *color = NULL;
    PyObject *text = NULL;
    uint8_t src = 0;

    base_data.type -> tp_init((PyObject *) self, NULL, NULL);
    self -> size = 50;
    self -> len = 4;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|UddddOb:Text", kwlist,
        &text, &self -> base.pos.x,
        &self -> base.pos.y, &self -> size,
        &self -> base.angle, &color,
        &src) || load(self, src))

    if (!text) {
        self -> content = realloc(self -> content, 5 * sizeof(wchar_t));

        if (!self -> content)
            return PyErr_NoMemory(), -1;

        wcscpy(self -> content, L"Text");
    }

    else INIT(content(self, text))
    return self -> content ? vector_set(color, (double *) &self -> base.color, 4) ? -1 : create(self) : (PyErr_NoMemory(), -1);
}

static PyObject *text_draw(Text *self, PyObject *args) {
    unbind();
    draw(self);

    Py_RETURN_NONE;
}

static PyObject *text_blit(Text *self, PyObject *item) {
    return screen_bind((Base *) self, item, (void (*)(Base *)) draw);
}

static void text_dealloc(Text *self) {
    glDeleteVertexArrays(1, &self -> vao);
    glDeleteBuffers(1, &self -> vbo);

    free(self -> content);
}

static PyObject *text_collide(Text *self, PyObject *item) {
    Vec2 poly[4];
    base_rect((Base *) self, poly, self -> width * self -> size, self -> size);

    return rect_intersect(item, poly);
}

static PyGetSetDef text_getset[] = {
    {"size", (getter) text_get_size, (setter) text_set_size, "The em units of the text", NULL},
    {"font", (getter) text_get_font, (setter) text_set_font, "The font of the text", NULL},
    {"content", (getter) text_get_content, (setter) text_set_content, "The content string of the text", NULL},
    {"width", (getter) text_get_width, NULL, "The measured width of the text", NULL},
    {"top", (getter) text_get_top, (setter) text_set_top, "The top position of the text", NULL},
    {"right", (getter) text_get_right, (setter) text_set_right, "The right position of the text", NULL},
    {"bottom", (getter) text_get_bottom, (setter) text_set_bottom, "The bottom position of the text", NULL},
    {"left", (getter) text_get_left, (setter) text_set_left, "The left position of the text", NULL},
    {NULL}
};

static PyMethodDef text_methods[] = {
    {"draw", (PyCFunction) text_draw, METH_NOARGS, "Draw the text on the screen"},
    {"blit", (PyCFunction) text_blit, METH_VARARGS, "Render the text to an offscreen surface"},
    {"collide", (PyCFunction) text_collide, METH_O, "Detect collision with another object"},
    {NULL}
};

static PyType_Slot text_slots[] = {
    {Py_tp_doc, "Render text on the screen"},
    {Py_tp_new, text_new},
    {Py_tp_init, text_init},
    {Py_tp_dealloc, text_dealloc},
    {Py_tp_getset, text_getset},
    {Py_tp_methods, text_methods},
    {0}
};

Spec text_data = {{"Text", sizeof(Text), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, text_slots}};