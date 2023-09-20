#include <main.h>

static size_t length(Circle *self) {
    return (size_t) (sqrt(fabs(self -> diameter / 2)) * 4) + 4;
}

static void data(Circle *self) {
    size_t total = length(self);
    size_t size = total * sizeof(GLfloat) * 2;
    GLfloat *data = malloc(size);

    data[x] = 0;
    data[y] = 0;

    for (size_t i = 1; i < total; i ++) {
        const double angle = M_PI * 2 * i / (total - 2);

        data[i * 2 + x] = cosf(angle) / 2;
        data[i * 2 + y] = sinf(angle) / 2;
    }

    glBindVertexArray(self -> vao);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);

    free(data);
}

static Sides sides(Circle *self) {
    double radius = Base_radius(&self -> base, self -> diameter);

    Vec2 pos = Circle_pos(self);
    Sides sides = {pos.y + radius, pos.y - radius, pos.x - radius, pos.x + radius};

    return sides;
}

static void unsafe(Circle *self) {
    cpCircleShapeSetRadius(self -> base.shape, Base_radius(&self -> base, self -> diameter));
    cpCircleShapeSetOffset(self -> base.shape, cpv(self -> base.transform.x, self -> base.transform.y));
}

static cpShape *physics(Circle *self) {
    return cpCircleShapeNew(self -> base.body -> body, Base_radius(&self -> base, self -> diameter), cpv(self -> base.transform.x, self -> base.transform.y));
}

static PyObject *Circle_get_diameter(Circle *self, void *closure) {
    return PyFloat_FromDouble(self -> diameter);
}

static int Circle_set_diameter(Circle *self, PyObject *value, void *closure) {
    DEL(value, "diameter")
    return ERR(self -> diameter = PyFloat_AsDouble(value)) ? -1 : (data(self), Base_unsafe(&self -> base), 0);
}

static PyObject *Circle_get_radius(Circle *self, void *closure) {
    return PyFloat_FromDouble(self -> diameter / 2);
}

static int Circle_set_radius(Circle *self, PyObject *value, void *closure) {
    DEL(value, "radius")

    const double radius = PyFloat_AsDouble(value);
    return ERR(radius) ? -1 : (self -> diameter = radius * 2, data(self), Base_unsafe(&self -> base), 0);
}

static PyObject *Circle_draw(Circle *self, PyObject *ignored) {
    Base_matrix(&self -> base, shape, self -> diameter, self -> diameter);

    glBindVertexArray(self -> vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, length(self));
    glBindVertexArray(0);
    Py_RETURN_NONE;
}

static Circle *Circle_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Circle *self = (Circle *) type -> tp_alloc(type, 0);

    if (self) {
        self -> base.sides = (Sides (*)(Base *)) sides;
        self -> base.physics = (cpShape *(*)(Base *)) physics;
        self -> base.unsafe = (void (*)(Base *)) unsafe;

        glGenVertexArrays(1, &self -> vao);
        glBindVertexArray(self -> vao);
        glGenBuffers(1, &self -> vbo);
        glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);

        glVertexAttribPointer(uniforms[vert], 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(uniforms[vert]);
        glBindVertexArray(0);
    }

    return self;
}

static int Circle_init(Circle *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"x", "y", "diameter", "color", NULL};

    PyObject *color = NULL;
    BaseType.tp_init((PyObject *) self, NULL, NULL);
    self -> diameter = 50;

    INIT(!PyArg_ParseTupleAndKeywords(
        args, kwds, "|dddO:Circle", kwlist, &self -> base.pos.x,
        &self -> base.pos.y, &self -> diameter, &color));

    return data(self), Vector_set(color, (vec) &self -> base.color, 4);
}

static void Circle_dealloc(Circle *self) {
    glDeleteBuffers(1, &self -> vbo);
    glDeleteVertexArrays(1, &self -> vao);
}

Vec2 Circle_pos(Circle *self) {
    const double sine = sin(self -> base.angle);
    const double cosine = cos(self -> base.angle);

    Vec2 pos = {
        self -> base.pos.x + self -> base.anchor.x * cosine - self -> base.anchor.y * sine,
        self -> base.pos.y + self -> base.anchor.y * cosine + self -> base.anchor.x * sine
    };

    return pos;
}

static PyGetSetDef Circle_getset[] = {
    {"diameter", (getter) Circle_get_diameter, (setter) Circle_set_diameter, "the diameter of the circle", NULL},
    {"radius", (getter) Circle_get_radius, (setter) Circle_set_radius, "the radius of the circle", NULL},
    {NULL}
};

static PyMethodDef Circle_methods[] = {
    {"draw", (PyCFunction) Circle_draw, METH_NOARGS, "draw the circle on the screen"},
    {NULL}
};

PyTypeObject CircleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Circle",
    .tp_doc = "draw circles on the screen",
    .tp_basicsize = sizeof(Circle),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc) Circle_new,
    .tp_init = (initproc) Circle_init,
    .tp_dealloc = (destructor) Circle_dealloc,
    .tp_base = &BaseType,
    .tp_methods = Circle_methods,
    .tp_getset = Circle_getset
};