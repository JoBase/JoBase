#include <main.h>
#include <chipmunk/chipmunk_unsafe.h>

static void new(Rectangle *self) {
    *self -> base.shapes = cpBoxShapeNew(self -> base.body, self -> size[x], self -> size[y], 0);
    self -> base.length = 1;
}

static void base(Rectangle *self) {
    if (!self -> base.length) return;

    const double sx = self -> size[x] / 2;
    const double sy = self -> size[y] / 2;

    cpVect data[4] = {{-sx, sy}, {sx, sy}, {sx, -sy}, {-sx, -sy}};
    cpPolyShapeSetVerts(*self -> base.shapes, 4, data, cpTransformNew(1, 0, 0, 1, 0, 0));
    baseMoment((Base *) self);
}

static cpFloat moment(Rectangle *self) {
    return cpMomentForBox(cpBodyGetMass(self -> base.body), self -> size[x], self -> size[y]);
}

static double top(Rectangle *self) {
    vec2 poly[4];
    return rectanglePoly(self, poly), getTop(poly, 4);
}

static double bottom(Rectangle *self) {
    vec2 poly[4];
    return rectanglePoly(self, poly), getBottom(poly, 4);
}

static double left(Rectangle *self) {
    vec2 poly[4];
    return rectanglePoly(self, poly), getLeft(poly, 4);
}

static double right(Rectangle *self) {
    vec2 poly[4];
    return rectanglePoly(self, poly), getRight(poly, 4);
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

static double Rectangle_vecSize(Rectangle *self, uint8_t index) {
    return self -> size[index];
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

static int Rectangle_init(Rectangle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "width", "height", "angle", "color", NULL};
    double angle = 0;

    PyObject *color = NULL;
    baseInit((Base *) self);

    self -> size[x] = 50;
    self -> size[y] = 50;

    int status = PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddddO", kwlist, &self -> base.pos[x], &self -> base.pos[y],
        &self -> size[x], &self -> size[y], &angle, &color);

    return !status || (color && vectorSet(color, self -> base.color, 4)) ? -1 : baseStart((Base *) self, angle), 0;
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

PyObject *rectangleNew(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Rectangle *self = (Rectangle *) baseNew(type, 1);

    self -> base.new = (void *)(Base *) new;
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

void rectanglePoly(Rectangle *self, poly poly) {
    const double px = self -> size[x] / 2;
    const double py = self -> size[y] / 2;

    poly[0][x] = poly[3][x] = self -> base.anchor[x] - px;
    poly[0][y] = poly[1][y] = self -> base.anchor[y] + py;
    poly[1][x] = poly[2][x] = self -> base.anchor[x] + px;
    poly[2][y] = poly[3][y] = self -> base.anchor[y] - py;

    rotate(poly, 4, cpBodyGetAngle(self -> base.body), self -> base.pos);
}

PyTypeObject RectangleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Rectangle",
    .tp_doc = "draw rectangles on the screen",
    .tp_basicsize = sizeof(Rectangle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = rectangleNew,
    .tp_init = (initproc) Rectangle_init,
    .tp_getset = RectangleGetSetters,
    .tp_methods = RectangleMethods
};