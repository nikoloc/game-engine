#include "triangle.h"

#include <stdbool.h>

#include "box.h"
#include "macros.h"

float
triangle_signed_area(vec2 a, vec2 b, vec2 c) {
    vec2 ac = vec2_sub(a, c);
    vec2 bc = vec2_sub(b, c);

    return (ac.x * bc.y - ac.y * bc.x) / 2.0f;
}

bool
triangle_contains_point(vec2 a, vec2 b, vec2 c, vec2 p) {
    float abp = triangle_signed_area(a, b, p);
    float bcp = triangle_signed_area(b, c, p);
    float cap = triangle_signed_area(c, a, p);

    return (abp >= 0 && bcp >= 0 && cap >= 0) || (abp <= 0 && bcp <= 0 && cap <= 0);
}

struct bounding_box
triangle_get_bounding_box(vec2 a, vec2 b, vec2 c) {
    return (struct bounding_box){
            floorf(min(a.x, min(b.x, c.x))),
            floorf(min(a.y, min(b.y, c.y))),
            ceilf(max(a.x, max(b.x, c.x))),
            ceilf(max(a.y, max(b.y, c.y))),
    };
}
