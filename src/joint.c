#include <main.h>

static void dealloc(Joint *self, Base *base) {
    if (base && base -> joint) {
        if (base -> joint -> src == self) {
            Array *joint = base -> joint;
            base -> joint = base -> joint -> next;

            free(joint);
        }

        else for (Array *this = base -> joint; this -> next; this = this -> next)
            if (this -> next -> src == self) {
                Array *joint = this -> next;
                this -> next = this -> next -> next;

                free(joint);
                break;
            }
    }
}

static void alloc(Joint *self, Base *base) {
    Py_INCREF(base);
    NEW(Array, base -> joint);

    base -> joint -> src = self;
}

static void delete(Joint *self) {
    if (self -> parent) {
        cpSpaceRemoveConstraint(self -> parent -> space, self -> joint);
        cpConstraintDestroy(self -> joint);
    }

    Py_CLEAR(self -> parent);
}

static void clean(Joint *self) {
    dealloc(self, self -> a);
    dealloc(self, self -> b);
    delete(self);

    Py_CLEAR(self -> a);
    Py_CLEAR(self -> b);
}

static int Joint_init(Joint *self, PyObject *args, PyObject *kwds) {
    self -> width = 2;

    self -> color.r = 0;
    self -> color.g = 0;
    self -> color.b = 0;
    self -> color.a = 1;

    return clean(self), 0;
}

static void Joint_dealloc(Joint *self) {
    GLuint buffers[] = {self -> vbo, self -> ibo};

    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &self -> vao);

    clean(self);
    cpConstraintFree(self -> joint);

    JointType.tp_free(self);
}

Joint *Joint_new(PyTypeObject *type, joint create, move unsafe, cpConstraint *joint) {
    Joint *self = (Joint *) type -> tp_alloc(type, 0);

    if (self) {
        Base_buffers(&self -> vao, &self -> vbo, &self -> ibo);

        self -> joint = joint;
        self -> create = create;
        self -> unsafe = unsafe;
    }

    return self;
}

void Joint_draw(Joint *self, Vec2 *base, size_t size) {
    GLfloat matrix[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

    glBindVertexArray(self -> vao);
    glBindBuffer(GL_ARRAY_BUFFER, self -> vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self -> ibo);

    Base_uniform(matrix, self -> color, shape);
    Line_create(base, size, self -> width);

    glDrawElements(GL_TRIANGLES, (size * 3 - 4) * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Joint_add(Joint *self) {
    alloc(self, self -> a);
    alloc(self, self -> b);

    Joint_check(self);
}

void Joint_check(Joint *self) {
    if (!self -> a -> body || !self -> b -> body || self -> a -> body -> parent != self -> b -> body -> parent || self -> a -> body == self -> b -> body)
        delete(self);

    else {
        if (self -> parent) {
            cpBody *a = cpConstraintGetBodyA(self -> joint);
            cpBody *b = cpConstraintGetBodyB(self -> joint);

            if (self -> a -> body -> body != a || self -> b -> body -> body != b)
                delete(self);
        }

        if (!self -> parent) {
            Py_INCREF(self -> parent = self -> a -> body -> parent);

            self -> create(self);
            Joint_unsafe(self);
            cpSpaceAddConstraint(self -> parent -> space, self -> joint);
        }
    }
}

void Joint_unsafe(Joint *self) {
    self -> unsafe(self);
}

cpVect Joint_rotate(Base *base, Vec2 pos) {
    const double angle = base -> rotate * M_PI / 180;
    return cpvadd(cpvrotate(cpv(pos.x, pos.y), cpvforangle(angle)), cpv(base -> transform.x, base -> transform.y));
}

PyTypeObject JointType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Joint",
    .tp_doc = "the base class for constraints between rigid bodies",
    .tp_basicsize = sizeof(Joint),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Joint_init,
    .tp_dealloc = (destructor) Joint_dealloc
};