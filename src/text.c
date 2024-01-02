#include <main.h>

static int create(Text *self) {
    if (FT_Set_Pixel_Sizes(self -> src -> face, (FT_UInt) self -> size, 0)) {
        PyErr_Format(PyExc_RuntimeError, "failed to set font size");
        return -1;
    }

    self -> descend = self -> src -> face -> size -> metrics.descender >> 6;
    self -> vect.y = self -> src -> face -> size -> metrics.height >> 6;
    self -> vect.x = 0;

    for (size_t i = 0; self -> content[i]; i ++) {
        wchar_t item = self -> content[i];
        FT_UInt index = FT_Get_Char_Index(self -> src -> face, item);
        Char *glyph = &self -> chars[index];

        if (glyph -> font != self -> size || !glyph -> src) {
            if (FT_Load_Glyph(self -> src -> face, index, FT_LOAD_DEFAULT)) {
                PyErr_Format(PyExc_RuntimeError, "failed to load glyph: \"%lc\"", item);
                return -1;
            }

            if (FT_Render_Glyph(self -> src -> face -> glyph, FT_RENDER_MODE_NORMAL)) {
                PyErr_Format(PyExc_RuntimeError, "failed to render glyph: \"%lc\"", item);
                return -1;
            }

            uint8_t *buffer = self -> src -> face -> glyph -> bitmap.buffer;
            FT_Glyph_Metrics metrics = self -> src -> face -> glyph -> metrics;

            glyph -> font = self -> size;
            glyph -> advance = metrics.horiAdvance >> 6;
            glyph -> size.x = metrics.width >> 6;
            glyph -> size.y = metrics.height >> 6;
            glyph -> pos.x = metrics.horiBearingX >> 6;
            glyph -> pos.y = metrics.horiBearingY >> 6;

            if (glyph -> src) glDeleteTextures(1, &glyph -> src);
            glGenTextures(1, &glyph -> src);

            glBindTexture(GL_TEXTURE_2D, glyph -> src);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, glyph -> size.x, glyph -> size.y, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (!i) self -> vect.x += glyph -> pos.x;
        if (self -> content[i + 1]) self -> vect.x += glyph -> advance;
        else self -> vect.x += glyph -> size.x + glyph -> pos.x;
    }

    self -> base.size.x = self -> vect.x;
    self -> base.size.y = self -> vect.y;

    return 0;
}

static void delete(Text *self) {
    for (FT_Long i = 0; i < self -> src -> face -> num_glyphs; i ++)
        if (self -> chars[i].src) {
            glDeleteTextures(1, &self -> chars[i].src);
            self -> chars[i].src = false;
        }
}

static void alloc(Text *self, Font *font) {
    self -> chars = realloc(self -> chars, font -> face -> num_glyphs * sizeof(Char));
    self -> src = font;
}

static int font(Text *self, const char *name) {
    FT_Face face;

    for (Font *this = fonts; this; this = this -> next)
        if (!strcmp(this -> name, name))
            return alloc(self, this), 0;

    if (FT_New_Face(library, name, 0, &face)) {
        PyErr_Format(PyExc_OSError, "failed to load font: \"%s\"", name);
        return -1;
    }

    NEW(Font, fonts)
    fonts -> name = strdup(name);
    fonts -> face = face;

    return alloc(self, fonts), 0;
}

static PyObject *Text_get_content(Text *self, void *closure) {
    return PyUnicode_FromWideChar(self -> content, -1);
}

static int Text_set_content(Text *self, PyObject *value, void *closure) {
    DEL(value, "content")

    const wchar_t *content = PyUnicode_AsWideCharString(value, NULL);
    INIT(!content)

    free(self -> content);
    return self -> content = wcsdup(content), create(self);
}

static PyObject *Text_get_font(Text *self, void *closure) {
    return PyUnicode_FromString(self -> src -> name);
}

static int Text_set_font(Text *self, PyObject *value, void *closure) {
    DEL(value, "font")
    delete(self);

    const char *name = PyUnicode_AsUTF8(value);
    return !name || font(self, name) ? -1 : create(self);
}

static PyObject *Text_get_font_size(Text *self, void *closure) {
    return PyFloat_FromDouble(self -> size);
}

static int Text_set_font_size(Text *self, PyObject *value, void *closure) {
    DEL(value, "font_size")
    return ERR(self -> size = PyFloat_AsDouble(value)) ? -1 : create(self);
}

static PyObject *Text_draw(Text *self, PyObject *args) {
    double pen = self -> base.base.anchor.x - self -> vect.x / 2;

    const double sx = self -> base.base.scale.x + self -> base.size.x / self -> vect.x - 1;
    const double sy = self -> base.base.scale.y + self -> base.size.y / self -> vect.y - 1;
    const double sine = sin(self -> base.base.angle * M_PI / 180);
    const double cosine = cos(self -> base.base.angle * M_PI / 180);

    glUniform1i(uniforms[img], text);
    glBindVertexArray(mesh);

    for (size_t i = 0; self -> content[i]; i ++) {
        wchar_t item = self -> content[i];

        Char glyph = self -> chars[FT_Get_Char_Index(self -> src -> face, item)];
        if (!i) pen -= glyph.pos.x;

        const double ax = pen + glyph.pos.x + glyph.size.x / 2;
        const double ay = self -> base.base.anchor.y + glyph.pos.y - (glyph.size.y + self -> vect.y) / 2 - self -> descend;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glyph.src);

        GLfloat matrix[] = {
            glyph.size.x * sx * cosine, glyph.size.x * sx * sine, 0,
            glyph.size.y * sy * -sine, glyph.size.y * sy * cosine, 0,
            ax * sx * cosine + ay * sy * -sine + self -> base.base.pos.x,
            ax * sx * sine + ay * sy * cosine + self -> base.base.pos.y, 1
        };

        Base_uniform(matrix, self -> base.base.color, text);
        pen += glyph.advance;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
    Py_RETURN_NONE;
}

static int Text_init(Text *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"content", "x", "y", "font_size", "angle", "color", "font", NULL};

    BaseType.tp_init((PyObject *) self, NULL, NULL);
    self -> size = 50;

    PyObject *content = NULL;
    PyObject *color = NULL;
    PyObject *src = PyObject_GetAttrString(module, "DEFAULT");
    INIT(!src)

    const char *file = PyUnicode_AsUTF8(src);
    Py_DECREF(src);

    INIT(!file || !PyArg_ParseTupleAndKeywords(
        args, kwds, "|UddddOs:Text", kwlist, &content, &self -> base.base.pos.x,
        &self -> base.base.pos.y, &self -> size, &self -> base.base.angle,
        &color, &file) || font(self, file) || Vector_set(color, (vec) &self -> base.base.color, 4))

    if (content) {
        wchar_t *text = PyUnicode_AsWideCharString(content, NULL);
        INIT(!text)

        self -> content = wcsdup(text);
    }

    else self -> content = wcsdup(L"Text");
    return create(self);
}

static void Text_dealloc(Text *self) {
    delete(self);
    free(self -> chars);
    free(self -> content);
}

static PyGetSetDef Text_getset[] = {
    {"content", (getter) Text_get_content, (setter) Text_set_content, "the message of the text", NULL},
    {"font", (getter) Text_get_font, (setter) Text_set_font, "file path to the font family", NULL},
    {"font_size", (getter) Text_get_font_size, (setter) Text_set_font_size, "size of the font", NULL},
    {NULL}
};

static PyMethodDef Text_methods[] = {
    {"draw", (PyCFunction) Text_draw, METH_NOARGS, "draw the text on the screen"},
    {NULL}
};

PyTypeObject TextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Text",
    .tp_doc = "draw text on the screen",
    .tp_basicsize = sizeof(Text),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &RectangleType,
    .tp_new = (newfunc) Rectangle_new,
    .tp_init = (initproc) Text_init,
    .tp_dealloc = (destructor) Text_dealloc,
    .tp_methods = Text_methods,
    .tp_getset = Text_getset
};