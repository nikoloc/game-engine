#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdbool.h>

#include "vec2.h"

float
triangle_signed_area(vec2 a, vec2 b, vec2 c);

bool
triangle_contains_point(vec2 a, vec2 b, vec2 c, vec2 p);

struct bounding_box
triangle_get_bounding_box(vec2 a, vec2 b, vec2 c);

#endif
