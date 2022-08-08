#include <glad/glad.h>
#include <main.h>
#include <chipmunk/chipmunk_unsafe.h>

static void new(Rectangle *self) {
    self -> base.shape = cpBoxShapeNew(
        self -> base.body,
        self -> size[x] * self -> base.scale[x],
        self -> size[y] * self -> base.scale[y], 0);
}

static void base(Rectangle *self) {
    if (!self -> base.shape) return;

    const double sx = self -> size[x] / 2;
    const double sy = self -> size[y] / 2;

    cpTransform shift = cpTransformNew(self -> base.scale[x], 0, 0, self -> base.scale[y], 0, 0);
    cpVect data[4] = {{-sx, sy}, {sx, sy}, {sx, -sy}, {-sx, -sy}};

    cpPolyShapeSetVerts(self -> base.shape, 4, data, shift);
    baseMoment((Base *) self);
}

static cpFloat moment(Rectangle *self) {
    return cpMomentForBox(
        self -> base.mass,
        self -> size[x] * self -> base.scale[x],
        self -> size[y] * self -> base.scale[y]);
}

static double top(Rectangle *self) {
    vec2 poly[4];
    return polyRect(self, poly), polyTop(poly, 4);
}

static double bottom(Rectangle *self) {
    vec2 poly[4];
    return polyRect(self, poly), polyBottom(poly, 4);
}

static double left(Rectangle *self) {
    vec2 poly[4];
    return polyRect(self, poly), polyLeft(poly, 4);
}

static double right(Rectangle *self) {
    vec2 poly[4];
    return polyRect(self, poly), polyRight(poly, 4);
}

static PyObject *Rectangle_getWidth(Rectangle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size[x]);
}

static int Rectangle_setWidth(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> size[x] = PyFloat_AsDouble(value);
    return ERR(self -> size[x]) ? -1 : base(self), 0;
}

static PyObject *Rectangle_getHeight(Rectangle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> size[1]);
}

static int Rectangle_setHeight(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> size[y] = PyFloat_AsDouble(value);
    return ERR(self -> size[y]) ? -1 : base(self), 0;
}

static vec Rectangle_vecSize(Rectangle *self) {
    return self -> size;
}

static PyObject *Rectangle_getSize(Rectangle *self, void *Py_UNUSED(closure)) {
    Vector *size = vectorNew((PyObject *) self, (Getter) Rectangle_vecSize, 2);

    size -> data[x].set = (setter) Rectangle_setWidth;
    size -> data[y].set = (setter) Rectangle_setHeight;
    size -> data[x].name = "x";
    size -> data[y].name = "y";

    return (PyObject *) size;
}

static int Rectangle_setSize(Rectangle *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> size, 2) ? -1 : base(self), 0;
}

static PyObject *Rectangle_draw(Rectangle *self, PyObject *Py_UNUSED(ignored)) {
    rectangleDraw(self, SHAPE);
    Py_RETURN_NONE;
}

static PyObject *Rectangle_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    return rectangleNew(type);
}

static int Rectangle_init(Rectangle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "width", "height", "angle", "color", NULL};

    PyObject *color = NULL;
    baseInit((Base *) self);

    self -> size[x] = 50;
    self -> size[y] = 50;

    int status = PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO", kwlist, &self -> base.pos[x], &self -> base.pos[y],
        &self -> size[x], &self -> size[y], &self -> base.angle, &color);

    return !status || (color && vectorSet(color, self -> base.color, 4)) ? -1 : 0;
}

static PyGetSetDef RectangleGetSetters[] = {
    {"width", (getter) Rectangle_getWidth, (setter) Rectangle_setWidth, "width of the rectangle", NULL},
    {"height", (getter) Rectangle_getHeight, (setter) Rectangle_setHeight, "height of the rectangle", NULL},
    {"size", (getter) Rectangle_getSize, (setter) Rectangle_setSize, "dimentions of the rectangle", NULL},
    {NULL}
};

static PyMethodDef RectangleMethods[] = {
    {"draw", (PyCFunction) Rectangle_draw, METH_NOARGS, "draw the rectangle on the screen"},
    {NULL}
};

PyObject *rectangleNew(PyTypeObject *type) {
    Rectangle *self = (Rectangle *) type -> tp_alloc(type, 0);

    self -> base.new = (void *)(Base *) new;
    self -> base.base = (void *)(Base *) base;
    self -> base.moment = (cpFloat (*)(Base *)) moment;
    self -> base.top = (double (*)(Base *)) top;
    self -> base.bottom = (double (*)(Base *)) bottom;
    self -> base.left = (double (*)(Base *)) left;
    self -> base.right = (double (*)(Base *)) right;

    return (PyObject *) self;
}

void rectangleDraw(Rectangle *self, uint8_t type) {
    baseMatrix((Base *) self, self -> size[x], self -> size[y]);

    glBindVertexArray(mesh);
    glUniform1i(uniform[img], type);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

PyTypeObject RectangleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Rectangle",
    .tp_doc = "draw rectangles on the screen",
    .tp_basicsize = sizeof(Rectangle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Rectangle_new,
    .tp_init = (initproc) Rectangle_init,
    .tp_getset = RectangleGetSetters,
    .tp_methods = RectangleMethods
};