#include "main.h"

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
    if (collide_circle_point(pos, radius, p1) || collide_circle_point(pos, radius, p2))
        return true;

    const double length = hypot(p1.x - p2.x, p1.y - p2.y);
    const double dot = ((pos.x - p1.x) * (p2.x - p1.x) + (pos.y - p1.y) * (p2.y - p1.y)) / pow(length, 2);
    Vec2 point = {p1.x + dot * (p2.x - p1.x), p1.y + dot * (p2.y - p1.y)};

    return segment_point(p1, p2, point) ? hypot(point.x - pos.x, point.y - pos.y) <= radius : 0;
}



bool collide_circle_point(Vec2 pos, double radius, Vec2 point) {
    return hypot(point.x - pos.x, point.y - pos.y) < radius;
}

bool collide_poly_point(Vec2 *poly, size_t size, Vec2 point) {
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

bool collide_poly_circle(Vec2 *poly, size_t size, Circle *circle) {
    const Vec2 pos = circle_pos(circle);
    const double rad = base_radius((Base *) circle, circle -> diameter);

    for (size_t i = 0; i < size; i ++)
        if (segment_circle(poly[i], poly[i + 1 == size ? 0 : i + 1], pos, rad))
            return true;

    return false;
}

int collide_line_line(Line *l1, Line *l2) {
    Vec2 *p1 = shape_points((Shape *) l1);

    if (p1) {
        Vec2 *p2 = shape_points((Shape *) l2);

        if (p2) {
            const double rad = base_radius((Base *) l1, l1 -> width) + base_radius((Base *) l2, l2 -> width);

            for (size_t i = 0; i < l1 -> base.len; i ++)
                for (size_t j = 0; j < l2 -> base.len; j ++)
                    if ((i && j && segment_segment(p1[i], p1[i - 1], p2[j], p2[j - 1])) ||
                        (i && segment_circle(p1[i], p1[i - 1], p2[j], rad)) ||
                        (j && segment_circle(p2[j], p2[j - 1], p1[i], rad))) return 1;
        }

        free(p1);
    }

    return -1;
}

int collide_line_point(Line *line, Vec2 point, double radius) {
    Vec2 *points = shape_points((Shape *) line);

    if (points) {
        const double rad = base_radius((Base *) line, line -> width) + radius;

        for (size_t i = 0; i < line -> base.len - 1; i ++)
            if (segment_circle(points[i], points[i + 1], point, rad))
                return free(points), 1;

        return free(points), 0;
    }

    return -1;
}

int collide_line_poly(Line *line, Vec2 *poly, size_t size) {
    Vec2 *points = shape_points((Shape *) line);

    if (points) {
        const double rad = base_radius((Base *) line, line -> width);

        if (collide_poly_point(poly, size, *points))
            return free(points), 1;

        for (size_t i = 0; i < line -> base.len; i ++)
            for (size_t j = 0; j < size; j ++) {
                Vec2 a = poly[j];
                Vec2 b = poly[j + 1 == size ? 0 : j + 1];

                if ((i && (segment_segment(points[i], points[i - 1], a, b) ||
                    segment_circle(points[i], points[i - 1], a, rad))) ||
                    segment_circle(a, b, points[i], rad)) return free(points), 1;
            }

        return free(points), 0;
    }

    return -1;
}

bool collide_poly_poly(Vec2 *p1, size_t s1, Vec2 *p2, size_t s2) {
    if (collide_poly_point(p1, s1, p2[0]) || collide_poly_point(p2, s2, p1[0]))
        return true;

    for (size_t i = 0; i < s1; i ++) {
        Vec2 last = p1[i + 1 == s1 ? 0 : i + 1];

        for (size_t j = 0; j < s2; j ++)
            if (segment_segment(p1[i], last, p2[j], p2[j + 1 == s2 ? 0 : j + 1]))
                return true;
    }

    return false;
}