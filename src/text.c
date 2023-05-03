#include <main.h>

static int reset(Text *text) {
    if (FT_Set_Pixel_Sizes(text -> font -> face, (FT_UInt) text -> size, 0))
        return format(PyExc_RuntimeError, "failed to set font size"), -1;

    text -> descend = text -> font -> face -> size -> metrics.descender >> 6;
    text -> base.y = text -> font -> face -> size -> metrics.height >> 6;
    text -> base.x = 0;

    for (size_t i = 0; text -> content[i]; i ++) {
        wchar_t item = text -> content[i];

        FT_UInt index = FT_Get_Char_Index(text -> font -> face, item);
        Char *glyph = &text -> chars[index];

        if (glyph -> font != (int) text -> size) {
            if (FT_Load_Glyph(text -> font -> face, index, FT_LOAD_DEFAULT))
                return format(PyExc_RuntimeError, "failed to load glyph: \"%lc\"", item), -1;

            if (FT_Render_Glyph(text -> font -> face -> glyph, FT_RENDER_MODE_NORMAL))
                return format(PyExc_RuntimeError, "failed to render glyph: \"%lc\"", item), -1;

            uint8_t *buffer = text -> font -> face -> glyph -> bitmap.buffer;
            FT_Glyph_Metrics metrics = text -> font -> face -> glyph -> metrics;

            glyph -> advance = metrics.horiAdvance >> 6;
            glyph -> size.x = metrics.width >> 6;
            glyph -> size.y = metrics.height >> 6;
            glyph -> pos.x = metrics.horiBearingX >> 6;
            glyph -> pos.y = metrics.horiBearingY >> 6;

            if (glyph -> load) glDeleteTextures(1, &glyph -> src);
            else glyph -> load = true;

            glGenTextures(1, &glyph -> src);
            glBindTexture(GL_TEXTURE_2D, glyph -> src);

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_R8, glyph -> size.x, glyph -> size.y,
                0, GL_RED, GL_UNSIGNED_BYTE, buffer);

            parameters();
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (!i) text -> base.x += glyph -> pos.x;
        if (text -> content[i + 1]) text -> base.x += glyph -> advance;
        else text -> base.x += glyph -> size.x + glyph -> pos.x;
    }

    text -> rect.size[x] = text -> base.x;
    text -> rect.size[y] = text -> base.y;

    return 0;
}

static void allocate(Text *text, Font *font) {
    text -> chars = realloc(text -> chars, font -> face -> num_glyphs * sizeof(Char));
    text -> font = font;

    FOR(FT_Long, font -> face -> num_glyphs)
        text -> chars[i].load = false;
}

static void delete(Text *text) {
    FOR(FT_Long, text -> font -> face -> num_glyphs)
        if (text -> chars[i].load) glDeleteTextures(1, &text -> chars[i].src);
}

static int font(Text *text, const char *name) {
    FT_Face face;

    for (Font *this = fonts; this; this = this -> next)
        if (!strcmp(this -> name, name)) return allocate(text, this), 0;

    if (FT_New_Face(library, name, 0, &face)) {
        return format(PyExc_FileNotFoundError, "failed to load font: \"%s\"", name), -1;
    }

    NEW(Font, fonts)
    fonts -> name = strdup(name);
    fonts -> face = face;

    return allocate(text, fonts), 0;
}

static PyObject *Text_getContent(Text *self, void *Py_UNUSED(closure)) {
    return PyUnicode_FromWideChar(self -> content, -1);
}

static int Text_setContent(Text *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    wchar_t *content = PyUnicode_AsWideCharString(value, NULL);
    if (!content) return -1;

    free(self -> content);
    return self -> content = wcsdup(content), reset(self);
}

static PyObject *Text_getFont(Text *self, void *Py_UNUSED(closure)) {
    return PyUnicode_FromString(self -> font -> name);
}

static int Text_setFont(Text *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)
    delete(self);

    const char *name = PyUnicode_AsUTF8(value);
    return !name || font(self, name) ? -1 : reset(self);
}

static PyObject *Text_getFontSize(Text *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size);
}

static int Text_setFontSize(Text *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> size = PyFloat_AsDouble(value);
    return ERR(self -> size) ? -1 : reset(self);
}

static PyObject *Text_draw(Text *self, PyObject *Py_UNUSED(ignored)) {
    double pen = self -> rect.base.anchor[x] - self -> base.x / 2;

    const double sx = self -> rect.base.scale[x] + self -> rect.size[x] / self -> base.x - 1;
    const double sy = self -> rect.base.scale[y] + self -> rect.size[y] / self -> base.y - 1;
    const double sine = sin(cpBodyGetAngle(self -> rect.base.body) * M_PI / 180);
	const double cosine = cos(cpBodyGetAngle(self -> rect.base.body) * M_PI / 180);
    const double px = self -> rect.base.pos[x];
    const double py = self -> rect.base.pos[y];

    glUniform1i(uniform[img], TEXT);
    glBindVertexArray(mesh);

    for (size_t i = 0; self -> content[i]; i ++) {
        wchar_t item = self -> content[i];

        Char glyph = self -> chars[FT_Get_Char_Index(self -> font -> face, item)];
        if (!i) pen -= glyph.pos.x;

        const double ax = pen + glyph.pos.x + glyph.size.x / 2;
        const double ay = self -> rect.base.anchor[y] + glyph.pos.y - (glyph.size.y + self -> base.y) / 2 - self -> descend;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glyph.src);

        mat matrix = {
            (GLfloat) (glyph.size.x * sx * cosine), (GLfloat) (glyph.size.x * sx * sine), 0, 0,
            (GLfloat) (glyph.size.y * sy * -sine), (GLfloat) (glyph.size.y * sy * cosine), 0, 0, 0, 0, 1, 0,
            (GLfloat) (ax * sx * cosine + ay * sy * -sine + px),
            (GLfloat) (ax * sx * sine + ay * sy * cosine + py), 0, 1
        };

        baseUniform(matrix, self -> rect.base.color);
        pen += glyph.advance;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
    Py_RETURN_NONE;
}

static int Text_init(Text *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"content", "x", "y", "font_size", "angle", "color", "font", NULL};
    const char *file = filepath("fonts/default.ttf");
    double angle = 0;

    PyObject *content = NULL;
    PyObject *color = NULL;

    baseInit((Base *) self);
    self -> size = 50;

    int state = PyArg_ParseTupleAndKeywords(
        args, kwds, "|UddddOs", kwlist, &content, &self -> rect.base.pos[x],
        &self -> rect.base.pos[y], &self -> size, &angle, &color, &file);

    if (!state || font(self, file) || (color && vectorSet(color, self -> rect.base.color, 4)))
        return -1;

    if (content) {
        wchar_t *text = PyUnicode_AsWideCharString(content, NULL);
        if (!text) return -1;

        self -> content = wcsdup(text);
    }

    else self -> content = wcsdup(L"Text");
    return baseStart((Base *) self, angle), reset(self);
}

static void Text_dealloc(Text *self) {
    if (self -> font) delete(self);

    free(self -> chars);
    free(self -> content);
    baseDealloc((Base *) self);
}

static PyGetSetDef TextGetSetters[] = {
    {"content", (getter) Text_getContent, (setter) Text_setContent, "message of the text", NULL},
    {"font", (getter) Text_getFont, (setter) Text_setFont, "file path to the font family", NULL},
    {"font_size", (getter) Text_getFontSize, (setter) Text_setFontSize, "size of the font", NULL},
    {NULL}
};

static PyMethodDef TextMethods[] = {
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
    .tp_new = rectangleNew,
    .tp_init = (initproc) Text_init,
    .tp_dealloc = (destructor) Text_dealloc,
    .tp_getset = TextGetSetters,
    .tp_methods = TextMethods
};