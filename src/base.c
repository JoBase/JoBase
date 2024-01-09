#include <main.h>

static bool circle_circle(Vec2 p1, Vec2 p2, double width) {
    return hypot(p2.x - p1.x, p2.y - p1.y) < width;
}

static bool circle_point(Vec2 pos, double radius, Vec2 point) {
    return hypot(point.x - pos.x, point.y - pos.y) < radius;
}

static bool segment_point(Vec2 p1, Vec2 p2, Vec2 point) {
    const double d1 = hypot(point.x - p1.x, point.y - p1.y);
    const double d2 = hypot(point.x - p2.x, point.y - p2.y);
    const double length = hypot(p1.x - p2.x, p1.y - p2.y);

    return d1 + d2 >= length - 0.1 && d1 + d2 <= length + 0.1;
}

static bool segment_segment(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4) {
    const double value = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
    const double u1 = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / value;
    const double u2 = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / value;

    return u1 >= 0 && u1 <= 1 && u2 >= 0 && u2 <= 1;
}

static bool segment_circle(Vec2 p1, Vec2 p2, Vec2 pos, double radius) {
    if (circle_point(pos, radius, p1) || circle_point(pos, radius, p2))
        return true;

    const double length = hypot(p1.x - p2.x, p1.y - p2.y);
    const double dot = ((pos.x - p1.x) * (p2.x - p1.x) + (pos.y - p1.y) * (p2.y - p1.y)) / pow(length, 2);
    Vec2 point = {p1.x + dot * (p2.x - p1.x), p1.y + dot * (p2.y - p1.y)};

    return segment_point(p1, p2, point) ? hypot(point.x - pos.x, point.y - pos.y) <= radius : 0;
}

static bool poly_segment(Vec2 *poly, size_t size, Vec2 p1, Vec2 p2) {
    for (size_t i = 0; i < size; i ++)
        if (segment_segment(p1, p2, poly[i], poly[i + 1 == size ? 0 : i + 1]))
            return true;

    return false;
}

static bool poly_point(Vec2 *poly, size_t size, Vec2 point) {
    bool hit = false;

    for (size_t i = 0; i < size; i ++) {
        Vec2 a = poly[i];
        Vec2 b = poly[i + 1 == size ? 0 : i + 1];

        if ((point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x) &&
            ((a.y > point.y && b.y < point.y) ||
            (a.y < point.y && b.y > point.y))) hit = !hit;
    }

    return hit;
}

static bool poly_poly(Vec2 *p1, size_t s1, Vec2 *p2, size_t s2) {
    if (poly_point(p1, s1, p2[0]) || poly_point(p2, s2, p1[0]))
        return true;

    for (size_t i = 0; i < s1; i ++)
        if (poly_segment(p2, s2, p1[i], p1[i + 1 == s1 ? 0 : i + 1]))
            return true;

    return false;
}

static bool poly_circle(Vec2 *poly, size_t size, Vec2 pos, double radius) {
    for (size_t i = 0; i < size; i ++)
        if (segment_circle(poly[i], poly[i + 1 == size ? 0 : i + 1], pos, radius))
            return true;

    return false;
}

static bool line_point(Vec2 *line, size_t size, double radius, Vec2 point) {
    for (size_t i = 0; i < size - 1; i ++)
        if (segment_circle(line[i], line[i + 1], point, radius))
            return true;

    return false;
}

static bool line_poly(Vec2 *line, size_t s1, double radius, Vec2 *poly, size_t s2) {
    if (poly_point(poly, s1, line[0]))
        return true;

    for (size_t i = 0; i < s1; i ++)
        for (size_t j = 0; j < s2; j ++) {
            Vec2 a = poly[j];
            Vec2 b = poly[j + 1 == s2 ? 0 : j + 1];

            if ((i && (segment_segment(line[i], line[i - 1], a, b) ||
                segment_circle(line[i], line[i - 1], a, radius))) ||
                segment_circle(a, b, line[i], radius)) return true;
        }

    return false;
}

static bool line_line(Vec2 *l1, size_t s1, Vec2 *l2, size_t s2, double width) {
    for (size_t i = 0; i < s1; i ++)
        for (size_t j = 0; j < s2; j ++)
            if ((i && j && segment_segment(l1[i], l1[i - 1], l2[j], l2[j - 1])) ||
                (i && segment_circle(l1[i], l1[i - 1], l2[j], width)) ||
                (j && segment_circle(l2[j], l2[j - 1], l1[i], width))) return true;

    return false;
}

static Body *clean(Base *self) {
    if (self -> body) {
        REM(Base, self -> body -> list, self)

        while (self -> shape) {
            cpShape *shape = self -> shape;
            self -> shape = cpShapeGetUserData(shape);

            cpSpaceRemoveShape(self -> body -> parent -> space, shape);
            cpShapeFree(shape);
        }
    }

    Body *body = self -> body;
    return self -> body = NULL, body;
}

static void group(Base *self, cpShape *shape) {
    cpGroup group = self -> group ? self -> group -> id : CP_NO_GROUP;
    cpShapeSetFilter(shape, cpShapeFilterNew(group, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES));
}

static int update(Base *self) {
    return Base_unsafe(self), 0;
}

static PyObject *Base_get_x(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.x);
}

static int Base_set_x(Base *self, PyObject *value, void *closure) {
    DEL(value, "x")
    return ERR(self -> pos.x = PyFloat_AsDouble(value)) ? -1 : update(self);
}

static PyObject *Base_get_y(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> pos.y);
}

static int Base_set_y(Base *self, PyObject *value, void *closure) {
    DEL(value, "y")
    return ERR(self -> pos.y = PyFloat_AsDouble(value)) ? -1 : update(self);
}

static Vector *Base_get_pos(Base *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> pos, 2, (set) update);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Base_set_pos(Base *self, PyObject *value, void *closure) {
    DEL(value, "pos")
    return Vector_set(value, (vec) &self -> pos, 2) ? -1 : update(self);
}

static Vector *Base_get_scale(Base *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> scale, 2, (set) update);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Base_set_scale(Base *self, PyObject *value, void *closure) {
    DEL(value, "scale")
    return Vector_set(value, (vec) &self -> scale, 2) ? -1 : update(self);
}

static Vector *Base_get_anchor(Base *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> anchor, 2, NULL);

    if (vector) {
        vector -> names[x] = 'x';
        vector -> names[y] = 'y';
    }

    return vector;
}

static int Base_set_anchor(Base *self, PyObject *value, void *closure) {
    DEL(value, "anchor")
    return Vector_set(value, (vec) &self -> anchor, 2);
}

static PyObject *Base_get_angle(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> angle);
}

static int Base_set_angle(Base *self, PyObject *value, void *closure) {
    DEL(value, "angle")
    return ERR(self -> angle = PyFloat_AsDouble(value)) ? -1 : update(self);
}

static PyObject *Base_get_red(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.r);
}

static int Base_set_red(Base *self, PyObject *value, void *closure) {
    DEL(value, "red")
    return ERR(self -> color.r = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *Base_get_green(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.g);
}

static int Base_set_green(Base *self, PyObject *value, void *closure) {
    DEL(value, "green")
    return ERR(self -> color.g = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *Base_get_blue(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.b);
}

static int Base_set_blue(Base *self, PyObject *value, void *closure) {
    DEL(value, "blue")
    return ERR(self -> color.b = PyFloat_AsDouble(value)) ? -1 : 0;
}

static PyObject *Base_get_alpha(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> color.a);
}

static int Base_set_alpha(Base *self, PyObject *value, void *closure) {
    DEL(value, "alpha")
    return ERR(self -> color.a = PyFloat_AsDouble(value)) ? -1 : 0;
}

static Vector *Base_get_color(Base *self, void *closure) {
    Vector *vector = Vector_new((PyObject *) self, (vec) &self -> color, 4, NULL);

    if (vector) {
        vector -> names[r] = 'r';
        vector -> names[g] = 'g';
        vector -> names[b] = 'b';
        vector -> names[a] = 'a';
    }

    return vector;
}

static int Base_set_color(Base *self, PyObject *value, void *closure) {
    DEL(value, "color")
    return Vector_set(value, (vec) &self -> color, 4);
}

static PyObject *Base_get_top(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> sides(self).top);
}

static int Base_set_top(Base *self, PyObject *value, void *closure) {
    DEL(value, "top")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.y += result - self -> sides(self).top, update(self));
}

static PyObject *Base_get_bottom(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> sides(self).bottom);
}

static int Base_set_bottom(Base *self, PyObject *value, void *closure) {
    DEL(value, "bottom")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.y += result - self -> sides(self).bottom, update(self));
}

static PyObject *Base_get_left(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> sides(self).left);
}

static int Base_set_left(Base *self, PyObject *value, void *closure) {
    DEL(value, "left")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.x += result - self -> sides(self).left, update(self));
}

static PyObject *Base_get_right(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> sides(self).right);
}

static int Base_set_right(Base *self, PyObject *value, void *closure) {
    DEL(value, "right")

    const double result = PyFloat_AsDouble(value);
    return ERR(result) ? -1 : (self -> pos.x += result - self -> sides(self).right, update(self));
}

static PyObject *Base_get_body(Base *self, void *closure) {
    if (self -> body)
        return Py_INCREF(self -> body), (PyObject *) self -> body;

    Py_RETURN_NONE;
}

static int Base_set_body(Base *self, PyObject *value, void *closure) {
    DEL(value, "body")
    Body *body = clean(self);

    if (value != Py_None) {
        if (Py_TYPE(value) != &BodyType) {
            PyErr_Format(PyExc_ValueError, "must be a Body, not %s", Py_TYPE(value) -> tp_name);
            return -1;
        }

        Py_INCREF(self -> body = (Body *) value);
        cpFloat angle = cpBodyGetAngle(self -> body -> body);
        cpVect pos = cpvunrotate(cpvsub(cpv(self -> pos.x, self -> pos.y), cpBodyGetPosition(self -> body -> body)), cpvforangle(angle));

        self -> transform.x = pos.x;
        self -> transform.y = pos.y;

        self -> rotate = self -> angle - angle * 180 / M_PI;
        self -> shape = self -> physics(self);

        for (cpShape *this = self -> shape; this; this = cpShapeGetUserData(this))
            Base_shape(self, this);

        self -> next = self -> body -> list;
        self -> body -> list = self;
    }

    for (Array *this = self -> joint; this; this = this -> next)
        Joint_check(this -> src);

    Py_CLEAR(body);
    return 0;
}

static PyObject *Base_get_group(Base *self, void *closure) {
    if (self -> group)
        return Py_INCREF(self -> group), (PyObject *) self -> group;

    Py_RETURN_NONE;
}

static int Base_set_group(Base *self, PyObject *value, void *closure) {
    DEL(value, "group")

    if (self -> group)
        Py_CLEAR(self -> group);

    if (value != Py_None) {
        if (Py_TYPE(value) != &GroupType) {
            PyErr_Format(PyExc_ValueError, "must be a Group, not %s", Py_TYPE(value) -> tp_name);
            return -1;
        }

        Py_INCREF(self -> group = (Group *) value);
    }

    for (cpShape *this = self -> shape; this; this = cpShapeGetUserData(this))
        group(self, this);

    return 0;
}

static PyObject *Base_get_mass(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> mass);
}

static int Base_set_mass(Base *self, PyObject *value, void *closure) {
    DEL(value, "mass")
    INIT(ERR(self -> mass = PyFloat_AsDouble(value)))

    return self -> body ? (cpShapeSetMass(self -> shape, self -> mass), 0) : 0;
}

static PyObject *Base_get_elasticity(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> elasticity);
}

static int Base_set_elasticity(Base *self, PyObject *value, void *closure) {
    DEL(value, "elasticity")
    INIT(ERR(self -> elasticity = PyFloat_AsDouble(value)))

    return self -> body ? (cpShapeSetElasticity(self -> shape, self -> elasticity), 0) : 0;
}

static PyObject *Base_get_friction(Base *self, void *closure) {
    return PyFloat_FromDouble(self -> friction);
}

static int Base_set_friction(Base *self, PyObject *value, void *closure) {
    DEL(value, "friction")
    INIT(ERR(self -> friction = PyFloat_AsDouble(value)))

    return self -> body ? (cpShapeSetFriction(self -> shape, self -> friction), 0) : 0;
}

// static PyObject *Base_look_at(Base *self, PyObject *value) {
//     double pos[2];

//     if (value == (PyObject *) cursor) {
//         pos[x] = cursor -> pos.x;
//         pos[y] = cursor -> pos.y;
//     }

//     else if (PyObject_TypeCheck(value, &BaseType)) {
//         pos[x] = ((Base *) value) -> pos.x;
//         pos[y] = ((Base *) value) -> pos.y;
//     }

//     else END(value)
//     self -> angle = atan2(pos[y] - self -> pos.y, pos[x] - self -> pos.x) * 180 / M_PI;

//     Base_unsafe(self);
//     Py_RETURN_NONE;
// }

static int Base_init(Base *self, PyObject *args, PyObject *kwds) {
    self -> angle = 0;
    self -> mass = 1;
    self -> elasticity = .5;
    self -> friction = .5;

    self -> scale.x = 1;
    self -> scale.y = 1;

    self -> anchor.x = 0;
    self -> anchor.y = 0;

    self -> pos.x = 0;
    self -> pos.y = 0;

    self -> color.r = 0;
    self -> color.g = 0;
    self -> color.b = 0;
    self -> color.a = 1;

    return Base_clean(self), 0;
}

static void Base_dealloc(Base *self) {
    Base_clean(self);
    BaseType.tp_free(self);
}

void Base_shape(Base *self, cpShape *shape) {
    cpShapeSetMass(shape, self -> mass);
    cpShapeSetElasticity(shape, self -> elasticity);
    cpShapeSetFriction(shape, self -> friction);
    cpSpaceAddShape(self -> body -> parent -> space, shape);
    group(self, shape);
}

void Base_uniform(GLfloat *matrix, Vec4 vect, uint8_t type) {
    glUniformMatrix3fv(uniforms[obj], 1, GL_FALSE, matrix);
    glUniform4f(uniforms[color], vect.r, vect.g, vect.b, vect.a);
    glUniform1i(uniforms[img], type);
}

void Base_matrix(Base *self, uint8_t type, double width, double height) {
    const double sine = sin(self -> angle * M_PI / 180);
    const double cosine = cos(self -> angle * M_PI / 180);

    GLfloat matrix[] = {
        width * self -> scale.x * cosine, width * self -> scale.x * sine, 0,
        height * self -> scale.y * -sine, height * self -> scale.y * cosine, 0,
        self -> anchor.x * cosine + self -> anchor.y * -sine + self -> pos.x,
        self -> anchor.x * sine + self -> anchor.y * cosine + self -> pos.y, 1
    };

    Base_uniform(matrix, self -> color, type);
}

void Base_unsafe(Base *self) {
    if (self -> body) {
        cpVect pos = cpBodyGetPosition(self -> body -> body);
        cpFloat angle = cpBodyGetAngle(self -> body -> body);
        cpVect rot = cpvforangle(-angle);

        pos.x = self -> pos.x - pos.x;
        pos.y = self -> pos.y - pos.y;

        self -> transform.x = pos.x * rot.x - pos.y * rot.y;
        self -> transform.y = pos.y * rot.x + pos.x * rot.y;
        self -> rotate = self -> angle - angle * 180 / M_PI;
        self -> unsafe(self);
    }

    for (Array *this = self -> joint; this; this = this -> next)
        Joint_unsafe(this -> src);
}

cpTransform Base_transform(Base *self) {
    cpVect rot = cpvforangle(self -> rotate * M_PI / 180);

    return cpTransformNewTranspose(
        rot.x * self -> scale.x, -rot.y, self -> transform.x,
        rot.y, rot.x * self -> scale.y, self -> transform.y);
}

Sides Base_sides(Base *self, Vec2 *points, size_t length) {
    const double cosine = cos(self -> angle * M_PI / 180);
    const double sine = sin(self -> angle * M_PI / 180);
    Sides sides;

    for (size_t i = 0; i < length; i ++) {
        const double px = points[i].x + self -> anchor.x;
        const double py = points[i].y + self -> anchor.y;
        const double rx = px * cosine - py * sine + self -> pos.x;
        const double ry = py * cosine + px * sine + self -> pos.y;

        sides.top = i && ry < sides.top ? sides.top : ry;
        sides.bottom = i && ry > sides.bottom ? sides.bottom : ry;
        sides.left = i && rx > sides.left ? sides.left : rx;
        sides.right = i && rx < sides.right ? sides.right : rx;
    }

    return sides;
}

void Base_clean(Base *self) {
    Body *body = clean(self);

    Py_CLEAR(self -> group);
    Py_CLEAR(body);
}

void Base_poly(Base *self, Vec2 *src, Vec2 *points, size_t length) {
    cpVect rot = cpvforangle(self -> angle * M_PI / 180);

    for (size_t i = 0; i < length; i ++) {
        const double px = src[i].x * self -> scale.x + self -> anchor.x;
        const double py = src[i].y * self -> scale.y + self -> anchor.y;

        points[i].x = px * rot.x - py * rot.y + self -> pos.x;
        points[i].y = py * rot.x + px * rot.y + self -> pos.y;
    }
}

double Base_radius(Base *self, double width) {
    return width / 2 * (self -> scale.x + self -> scale.y) / 2;
}

void Base_buffers(GLuint *vao, GLuint *vbo, GLuint *ibo) {
    GLuint buffers[2];

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    glGenBuffers(2, buffers);

    *vbo = buffers[0];
    *ibo = buffers[1];

    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
    glVertexAttribPointer(uniforms[vert], 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(uniforms[vert]);
    glBindVertexArray(0);
}

PyObject *Base_collide(PyObject *self, PyObject *other) {
    bool result;

    if (PyObject_TypeCheck(self, &RectangleType)) {
        Vec2 a[4];
        Rectangle_poly((Rectangle *) self, a);

        if (PyObject_TypeCheck(other, &RectangleType)) {
            Vec2 b[4];
            Rectangle_poly((Rectangle *) other, b);

            result = poly_poly(a, 4, b, 4);
        }

        else if (PyObject_TypeCheck(other, &CircleType)) {
            Circle *b = (Circle *) other;
            Vec2 pos = Circle_pos(b);

            result = poly_circle(a, 4, pos, Base_radius(&b -> base, b -> diameter));
        }

        else if (PyObject_TypeCheck(other, &LineType)) {
            Line *line = (Line *) other;
            Vec2 *b = malloc(line -> base.length * sizeof(Vec2));

            Shape_poly(&line -> base, b);
            result = line_poly(b, line -> base.length, Base_radius(&line -> base.base, line -> width), a, 4);
            free(b);
        }

        else if (PyObject_TypeCheck(other, &ShapeType)) {
            Shape *shape = (Shape *) other;
            Vec2 *b = malloc(shape -> length * sizeof(Vec2));

            Shape_poly(shape, b);
            result = poly_poly(a, 4, b, shape -> length);
            free(b);
        }

        else if (Py_TYPE(other) == &CursorType)
            result = poly_point(a, 4, cursor -> pos);

        else END(other)
    }

    else if (PyObject_TypeCheck(self, &CircleType)) {
        Circle *a = (Circle *) self;
        Vec2 pos = Circle_pos(a);

        if (PyObject_TypeCheck(other, &RectangleType)) {
            Vec2 b[4];
            Rectangle_poly((Rectangle *) other, b);

            result = poly_circle(b, 4, pos, Base_radius(&a -> base, a -> diameter));
        }

        else if (PyObject_TypeCheck(other, &CircleType)) {
            Circle *b = (Circle *) other;
            Vec2 point = Circle_pos(b);

            result = circle_circle(pos, point, Base_radius(&a -> base, a -> diameter) + Base_radius(&b -> base, b -> diameter));
        }

        else if (PyObject_TypeCheck(other, &LineType)) {
            Line *line = (Line *) other;
            Vec2 *b = malloc(line -> base.length * sizeof(Vec2));

            Shape_poly(&line -> base, b);
            result = line_point(b, line -> base.length, Base_radius(&line -> base.base, line -> width) + Base_radius(&a -> base, a -> diameter), pos);
            free(b);
        }

        else if (PyObject_TypeCheck(other, &ShapeType)) {
            Shape *shape = (Shape *) other;
            Vec2 *b = malloc(shape -> length * sizeof(Vec2));

            Shape_poly(shape, b);
            result = poly_circle(b, shape -> length, pos, Base_radius(&a -> base, a -> diameter));
            free(b);
        }

        else if (Py_TYPE(other) == &CursorType)
            result = circle_point(pos, Base_radius(&a -> base, a -> diameter), cursor -> pos);

        else END(other)
    }

    else if (PyObject_TypeCheck(self, &LineType)) {
        Line *line = (Line *) self;
        Vec2 *a = malloc(line -> base.length * sizeof(Vec2));

        const double radius = Base_radius(&line -> base.base, line -> width);
        Shape_poly(&line -> base, a);

        if (PyObject_TypeCheck(other, &RectangleType)) {
            Vec2 b[4];
            Rectangle_poly((Rectangle *) other, b);

            result = line_poly(a, line -> base.length, radius, b, 4);
        }

        else if (PyObject_TypeCheck(other, &CircleType)) {
            Circle *b = (Circle *) other;
            Vec2 pos = Circle_pos(b);

            result = line_point(a, line -> base.length, radius + Base_radius(&b -> base, b -> diameter), pos);
        }

        else if (PyObject_TypeCheck(other, &LineType)) {
            Line *object = (Line *) other;
            Vec2 *b = malloc(object -> base.length * sizeof(Vec2));

            Shape_poly(&object -> base, b);
            result = line_line(a, line -> base.length, b, object -> base.length, radius + Base_radius(&object -> base.base, object -> width));
            free(b);
        }

        else if (PyObject_TypeCheck(other, &ShapeType)) {
            Shape *shape = (Shape *) other;
            Vec2 *b = malloc(shape -> length * sizeof(Vec2));

            Shape_poly(shape, b);
            result = line_poly(a, line -> base.length, radius, b, shape -> length);
            free(b);
        }

        else if (Py_TYPE(other) == &CursorType)
            result = line_point(a, line -> base.length, radius, cursor -> pos);

        else {
            free(a);
            END(other)
        }

        free(a);
    }

    else if (PyObject_TypeCheck(self, &ShapeType)) {
        Shape *shape = (Shape *) self;
        Vec2 *a = malloc(shape -> length * sizeof(Vec2));
        Shape_poly(shape, a);

        if (PyObject_TypeCheck(other, &RectangleType)) {
            Vec2 b[4];
            Rectangle_poly((Rectangle *) other, b);

            result = poly_poly(a, shape -> length, b, 4);
        }

        else if (PyObject_TypeCheck(other, &CircleType)) {
            Circle *b = (Circle *) other;
            Vec2 pos = Circle_pos(b);

            result = poly_circle(a, shape -> length, pos, Base_radius(&b -> base, b -> diameter));
        }

        else if (PyObject_TypeCheck(other, &LineType)) {
            Line *line = (Line *) other;
            Vec2 *b = malloc(line -> base.length * sizeof(Vec2));

            Shape_poly(&line -> base, b);
            result = line_poly(b, line -> base.length, Base_radius(&line -> base.base, line -> width), a, shape -> length);
            free(b);
        }

        else if (PyObject_TypeCheck(other, &ShapeType)) {
            Shape *object = (Shape *) other;
            Vec2 *b = malloc(object -> length * sizeof(Vec2));

            Shape_poly(object, b);
            result = poly_poly(a, shape -> length, b, object -> length);
            free(b);
        }

        else if (Py_TYPE(other) == &CursorType)
            result = poly_point(a, shape -> length, cursor -> pos);

        else {
            free(a);
            END(other)
        }

        free(a);
    }

    else if (self == (PyObject *) cursor) {
        if (PyObject_TypeCheck(other, &RectangleType)) {
            Vec2 b[4];
            Rectangle_poly((Rectangle *) other, b);

            result = poly_point(b, 4, cursor -> pos);
        }

        else if (PyObject_TypeCheck(other, &CircleType)) {
            Circle *b = (Circle *) other;
            Vec2 point = Circle_pos(b);

            result = circle_point(point, Base_radius(&b -> base, b -> diameter), cursor -> pos);
        }

        else if (PyObject_TypeCheck(other, &LineType)) {
            Line *line = (Line *) other;
            Vec2 *b = malloc(line -> base.length * sizeof(Vec2));

            Shape_poly(&line -> base, b);
            result = line_point(b, line -> base.length, Base_radius(&line -> base.base, line -> width), cursor -> pos);
            free(b);
        }

        else if (PyObject_TypeCheck(other, &ShapeType)) {
            Shape *shape = (Shape *) other;
            Vec2 *b = malloc(shape -> length * sizeof(Vec2));

            Shape_poly(shape, b);
            result = poly_point(b, shape -> length, cursor -> pos);
            free(b);
        }

        else if (Py_TYPE(other) == &CursorType)
            Py_RETURN_TRUE;

        else END(other)
    }

    else END(self)
    return PyBool_FromLong(result);
}

static PyGetSetDef Base_getset[] = {
    {"x", (getter) Base_get_x, (setter) Base_set_x, "x position of the object", NULL},
    {"y", (getter) Base_get_y, (setter) Base_set_y, "y position of the object", NULL},
    {"position", (getter) Base_get_pos, (setter) Base_set_pos, "position of the object", NULL},
    {"pos", (getter) Base_get_pos, (setter) Base_set_pos, "position of the object", NULL},
    {"scale", (getter) Base_get_scale, (setter) Base_set_scale, "scale of the object", NULL},
    {"anchor", (getter) Base_get_anchor, (setter) Base_set_anchor, "rotation origin of the object", NULL},
    {"angle", (getter) Base_get_angle, (setter) Base_set_angle, "angle of the object", NULL},
    {"red", (getter) Base_get_red, (setter) Base_set_red, "red color of the object", NULL},
    {"green", (getter) Base_get_green, (setter) Base_set_green, "green color of the object", NULL},
    {"blue", (getter) Base_get_blue, (setter) Base_set_blue, "blue color of the object", NULL},
    {"alpha", (getter) Base_get_alpha, (setter) Base_set_alpha, "opacity of the object", NULL},
    {"color", (getter) Base_get_color, (setter) Base_set_color, "color of the object", NULL},
    {"top", (getter) Base_get_top, (setter) Base_set_top, "top position of the object", NULL},
    {"bottom", (getter) Base_get_bottom, (setter) Base_set_bottom, "bottom position of the object", NULL},
    {"left", (getter) Base_get_left, (setter) Base_set_left, "left position of the object", NULL},
    {"right", (getter) Base_get_right, (setter) Base_set_right, "right position of the object", NULL},
    {"body", (getter) Base_get_body, (setter) Base_set_body, "the attached rigid body", NULL},
    {"group", (getter) Base_get_group, (setter) Base_set_group, "the attached collision group", NULL},
    {"mass", (getter) Base_get_mass, (setter) Base_set_mass, "the weight of the object in a physics environment", NULL},
    {"weight", (getter) Base_get_mass, (setter) Base_set_mass, "the weight of the object in a physics environment", NULL},
    {"elasticity", (getter) Base_get_elasticity, (setter) Base_set_elasticity, "the bounciness of the object in a physics environment", NULL},
    {"friction", (getter) Base_get_friction, (setter) Base_set_friction, "the rougness of the object in a physics environment", NULL},
    {NULL}
};

static PyMethodDef Base_methods[] = {
    {"collides_with", (PyCFunction) Base_collide, METH_O, "detect if two objects collide"},
    {"collide", (PyCFunction) Base_collide, METH_O, "detect if two objects collide"},
    // {"look_at", (PyCFunction) Base_look_at, METH_O, "rotate the object so that it faces another object"},
    {NULL}
};

PyTypeObject BaseType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Base",
    .tp_doc = "original class for drawing things",
    .tp_basicsize = sizeof(Base),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) Base_init,
    .tp_dealloc = (destructor) Base_dealloc,
    .tp_methods = Base_methods,
    .tp_getset = Base_getset
};