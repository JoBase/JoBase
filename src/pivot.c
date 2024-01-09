// #include <main.h>

// static void unsafe(Pivot *self) {
//     cpPivotJointSetAnchorA(self -> base.joint, Joint_rotate(self -> base.a, self -> start));
//     cpPivotJointSetAnchorB(self -> base.joint, Joint_rotate(self -> base.b, self -> end));
// }

// static int anchor(Pivot *self) {
//     if (self -> base.parent) unsafe(self);
//     return 0;
// }

// static void create(Joint *self) {
//     cpPivotJointInit((cpPivotJoint *) self -> joint, self -> a -> body -> body, self -> b -> body -> body, cpv(0, 0), cpv(0, 0));
// }

// static Vector *Pivot_get_start(Pivot *self, void *closure) {
//     Vector *vector = Vector_new((PyObject *) self, (vec) &self -> start, 2, (set) anchor);

//     if (vector) {
//         vector -> names[x] = 'x';
//         vector -> names[y] = 'y';
//     }

//     return vector;
// }

// static int Pivot_set_start(Pivot *self, PyObject *value, void *closure) {
//     DEL(value, "start")
//     return Vector_set(value, (vec) &self -> start, 2) ? -1 : anchor(self);
// }

// static Vector *Pivot_get_end(Pivot *self, void *closure) {
//     Vector *vector = Vector_new((PyObject *) self, (vec) &self -> end, 2, (set) anchor);

//     if (vector) {
//         vector -> names[x] = 'x';
//         vector -> names[y] = 'y';
//     }

//     return vector;
// }

// static int Pivot_set_end(Pivot *self, PyObject *value, void *closure) {
//     DEL(value, "end")
//     return Vector_set(value, (vec) &self -> end, 2) ? -1 : anchor(self);
// }

// static PyObject *Pivot_draw(Pivot *self, PyObject *args) {
//     if (Joint_active(&self -> base)) {
//         Vec2 base[] = {
//             Body_get(self -> base.a -> body, Joint_rotate(self -> base.a, self -> start)),
//             Body_get(self -> base.b -> body, Joint_rotate(self -> base.b, self -> end))
//         };

//         Joint_draw(&self -> base, base, 2);
//     }

//     Py_RETURN_NONE;
// }

// static Joint *Pivot_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
//     return Joint_new(type, (joint) create, (move) unsafe, (cpConstraint *) cpPivotJointAlloc());
// }

// static int Pivot_init(Pivot *self, PyObject *args, PyObject *kwds) {
//     static char *kwlist[] = {"a", "b", "start", "end", "width", "color", NULL};

//     PyObject *color = NULL;
//     PyObject *start = NULL;
//     PyObject *end = NULL;
//     JointType.tp_init((PyObject *) self, NULL, NULL);

//     self -> start.x = 50;
//     self -> start.y = 50;
//     self -> end.x = 0;
//     self -> end.y = 0;

//     INIT(!PyArg_ParseTupleAndKeywords(
//         args, kwds, "O!O!|OOddO:Pivot", kwlist, &BaseType, &self -> base.a,
//         &BaseType, &self -> base.b, &start, &end, &self -> base.width,
//         &color) || Vector_set(color, (vec) &self -> base.color, 4) || Vector_set(start, (vec) &self -> start, 2) || Vector_set(end, (vec) &self -> end, 2))

//     return Joint_add(&self -> base), 0;
// }

// static PyGetSetDef Pivot_getset[] = {
//     {"start", (getter) Pivot_get_start, (setter) Pivot_set_start, "the offset of the pivot relative to the first body", NULL},
//     {"end", (getter) Pivot_get_end, (setter) Pivot_set_end, "the offset of the pivot relative to the last body", NULL},
//     {NULL}
// };

// static PyMethodDef Pivot_methods[] = {
//     {"draw", (PyCFunction) Pivot_draw, METH_NOARGS, "draw the pivot joint on the screen"},
//     {NULL}
// };

// PyTypeObject PivotType = {
//     PyVarObject_HEAD_INIT(NULL, 0)
//     .tp_name = "Pivot",
//     .tp_doc = "Attach two bodies at a certain point",
//     .tp_basicsize = sizeof(Pivot),
//     .tp_itemsize = 0,
//     .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
//     .tp_base = &JointType,
//     .tp_new = (newfunc) Pivot_new,
//     .tp_init = (initproc) Pivot_init,
//     .tp_methods = Pivot_methods,
//     .tp_getset = Pivot_getset
// };