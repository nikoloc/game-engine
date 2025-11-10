#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdbool.h>

#include "vec2.h"
#include "vec3.h"

struct triangle2 {
    vec2 a, b, c;
};

struct triangle3 {
    vec3 a, b, c;
};

float
triangle2_signed_area(struct triangle2 *triangle);

bool
triangle2_contains_point(struct triangle2 *t, vec2 p);

struct bounding_box
triangle2_get_bounding_box(struct triangle2 *t);

#endif
