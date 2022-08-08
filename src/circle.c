#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <main.h>
#include <chipmunk/chipmunk_unsafe.h>

static size_t vertices(Circle *self) {
    return (int) (sqrt(fabs(self -> radius * AVR(self -> base.scale))) * 4) + 4;
}

static void data(Circle *self) {
    size_t total = vertices(self);
    size_t size = total * sizeof(GLfloat) * 2;
    GLfloat *data = malloc(size);

    data[x] = 0;
    data[y] = 0;

    FOR(size_t, total - 1) {
        const double angle = M_PI * 2 * i / (total - 2);

        data[i * 2 + 2] = (GLfloat) cos(angle) / 2;
        data[i * 2 + 3] = (GLfloat) sin(angle) / 2;
    }

    glBindVertexArray(self -> vao);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
}

static void new(Circle *self) {
    self -> base.shape = cpCircleShapeNew(self -> base.body, self -> radius * AVR(self -> base.scale), cpv(0, 0));
}

static void base(Circle *self) {
    data(self);

    if (self -> base.shape) {
        baseMoment((Base *) self);
        cpCircleShapeSetRadius(self -> base.shape, self -> radius * AVR(self -> base.scale));
    }
}

static cpFloat moment(Circle *self) {
    return cpMomentForCircle(self -> base.mass, 0, self -> radius * AVR(self -> base.scale), cpv(0, 0));
}

static double top(Circle *self) {
    return circleY(self) + self -> radius * AVR(self -> base.scale);
}

static double bottom(Circle *self) {
    return circleY(self) - self -> radius * AVR(self -> base.scale);
}

static double left(Circle *self) {
    return circleX(self) - self -> radius * AVR(self -> base.scale);
}

static double right(Circle *self) {
    return circleX(self) + self -> radius * AVR(self -> base.scale);
}

static PyObject *Circle_getDiameter(Circle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> radius * 2);
}

static int Circle_setDiameter(Circle *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    const double diameter = PyFloat_AsDouble(value);
    if (ERR(diameter)) return -1;

    self -> radius = diameter / 2;
    return data(self), 0;
}

static PyObject *Circle_getRadius(Circle *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> radius);
}

static int Circle_setRadius(Circle *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> radius = PyFloat_AsDouble(value);
    return ERR(self -> radius) ? -1 : data(self), 0;
}

static PyObject *Circle_draw(Circle *self, PyObject *Py_UNUSED(ignored)) {
    baseMatrix((Base *) self, self -> radius * 2, self -> radius * 2);

    glBindVertexArray(self -> vao);
    glUniform1i(uniform[img], SHAPE);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices(self));

    glBindVertexArray(0);
    Py_RETURN_NONE;
}

static PyObject *Circle_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    Circle *self = (Circle *) type -> tp_alloc(type, 0);

    self -> base.new = (void *)(Base *) new;
    self -> base.base = (void *)(Base *) base;
    self -> base.moment = (cpFloat (*)(Base *)) moment;
    self -> base.top = (double (*)(Base *)) top;
    self -> base.bottom = (double (*)(Base *)) bottom;
    self -> base.left = (double (*)(Base *)) left;
    self -> base.right = (double (*)(Base *)) right;

    glGenVertexArrays(1, &self -> vao);
    glBindVertexArray(self -> vao);
    glGenBuffers(1, &self -> vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);

    glVertexAttribPointer(uniform[vert], 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(uniform[vert]);
    glBindVertexArray(0);

    return (PyObject *) self;
}

static int Circle_init(Circle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "diameter", "color", NULL};
    double diameter = 50;

    PyObject *color = NULL;
    baseInit((Base *) self);

    int state = PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddO", kwlist, &self -> base.pos[x],
        &self -> base.pos[y], &diameter, &color);

    if (!state || (color && vectorSet(color, self -> base.color, 4)))
        return -1;

    self -> radius = diameter / 2;
    return data(self), 0;
}

static void Circle_dealloc(Circle *self) {
    glDeleteBuffers(1, &self -> vbo);
    glDeleteVertexArrays(1, &self -> vao);

    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyGetSetDef CircleGetSetters[] = {
    {"diameter", (getter) Circle_getDiameter, (setter) Circle_setDiameter, "diameter of the circle", NULL},
    {"radius", (getter) Circle_getRadius, (setter) Circle_setRadius, "radius of the circle", NULL},
    {NULL}
};

static PyMethodDef CircleMethods[] = {
    {"draw", (PyCFunction) Circle_draw, METH_NOARGS, "draw the circle on the screen"},
    {NULL}
};

double circleX(Circle *self) {
    const double angle = self -> base.angle * M_PI / 180;
    return self -> base.pos[x] + self -> base.anchor[x] * cos(angle) - self -> base.anchor[y] * sin(angle);
}

double circleY(Circle *self) {
    const double angle = self -> base.angle * M_PI / 180;
    return self -> base.pos[y] + self -> base.anchor[y] * cos(angle) + self -> base.anchor[x] * sin(angle);
}

PyTypeObject CircleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Circle",
    .tp_doc = "draw circles on the screen",
    .tp_basicsize = sizeof(Circle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_base = &BaseType,
    .tp_new = Circle_new,
    .tp_init = (initproc) Circle_init,
    .tp_dealloc = (destructor) Circle_dealloc,
    .tp_getset = CircleGetSetters,
    .tp_methods = CircleMethods
};