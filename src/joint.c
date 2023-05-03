#include <main.h>

static PyObject *Joint_getRed(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[r]);
}

static int Joint_setRed(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> color[r] = PyFloat_AsDouble(value);
    return ERR(self -> color[r]) ? -1 : 0;
}

static PyObject *Joint_getGreen(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[g]);
}

static int Joint_setGreen(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> color[g] = PyFloat_AsDouble(value);
    return ERR(self -> color[g]) ? -1 : 0;
}

static PyObject *Joint_getBlue(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[b]);
}

static int Joint_setBlue(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> color[b] = PyFloat_AsDouble(value);
    return ERR(self -> color[b]) ? -1 : 0;
}

static PyObject *Joint_getAlpha(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> color[a]);
}

static int Joint_setAlpha(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> color[a] = PyFloat_AsDouble(value);
    return ERR(self -> color[a]) ? -1 : 0;
}

static double Joint_vecColor(Joint *self, uint8_t index) {
    return self -> color[index];
}

static PyObject *Joint_getColor(Joint *self, void *Py_UNUSED(closure)) {
    Vector *color = vectorNew((PyObject *) self, (Getter) Joint_vecColor, 4);

    color -> data[r].set = (setter) Joint_setRed;
    color -> data[g].set = (setter) Joint_setGreen;
    color -> data[b].set = (setter) Joint_setBlue;
    color -> data[a].set = (setter) Joint_setAlpha;
    color -> data[r].name = "r";
    color -> data[g].name = "g";
    color -> data[b].name = "b";
    color -> data[a].name = "a";

    return (PyObject *) color;
}

static int Joint_setColor(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    return vectorSet(value, self -> color, 4);
}

static PyObject *Joint_getWidth(Joint *self, void *Py_UNUSED(closure)) {
    return PyFloat_FromDouble(self -> width);
}

static int Joint_setWidth(Joint *self, PyObject *value, void *Py_UNUSED(closure)) {
    DEL(value)

    self -> width = PyFloat_AsDouble(value);
    return ERR(self -> width) ? -1 : 0;
}

static void Joint_dealloc(Joint *self) {
    GLuint buffers[] = {self -> vbo, self -> ibo};

    Py_DECREF(self -> a);
    Py_DECREF(self -> b);

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);

    cpConstraintFree(self -> joint);
    Py_TYPE(self) -> tp_free((PyObject *) self);
}

static PyGetSetDef JointGetSetters[] = {
    {"red", (getter) Joint_getRed, (setter) Joint_setRed, "red color of the joint", NULL},
    {"green", (getter) Joint_getGreen, (setter) Joint_setGreen, "green color of the joint", NULL},
    {"blue", (getter) Joint_getBlue, (setter) Joint_setBlue, "blue color of the joint", NULL},
    {"alpha", (getter) Joint_getAlpha, (setter) Joint_setAlpha, "opacity of the joint", NULL},
    {"color", (getter) Joint_getColor, (setter) Joint_setColor, "color of the joint", NULL},
    {"width", (getter) Joint_getWidth, (setter) Joint_setWidth, "thickness of the joint", NULL},
    {NULL}
};

void jointDraw(Joint *self, poly base, size_t size) {
    mat matrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    baseUniform(matrix, self -> color);

    glBindVertexArray(self -> vao);
    glUniform1i(uniform[img], SHAPE);

    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

    lineCreate(base, size, self -> width);
    glDrawElements(GL_TRIANGLES, IDX(size * 3 - 2), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void jointInit(Joint *self) {
    self -> color[r] = self -> color[g] = self -> color[b] = 0;
    self -> color[a] = 1;
    self -> width = 2;
}

int jointStart(Joint *self, PyObject *color) {
    if (color && vectorSet(color, self -> color, 4))
        return -1;

    Py_INCREF(self -> a);
    Py_INCREF(self -> b);
    return 0;
}

PyObject *jointNew(PyTypeObject *type, cpConstraint *joint) {
    Joint *self = (Joint *) type -> tp_alloc(type, 0);
    buffers(&self -> vao, &self -> vbo, &self -> ibo);
    return self -> joint = joint, (PyObject *) self;
}

PyTypeObject JointType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Joint",
    .tp_doc = "base class for physics constraints between bodies",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) Joint_dealloc,
    .tp_getset = JointGetSetters
};