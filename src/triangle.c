#include "triangle.h"

#include <stdbool.h>

#include "box.h"
#include "macros.h"

float
triangle2_signed_area(struct triangle2 *t) {
    vec2 ac = vec2_sub(t->a, t->c);
    vec2 bc = vec2_sub(t->b, t->c);

    return (ac.x * bc.y - ac.y * bc.x) / 2.0f;
}

bool
triangle2_contains_point(struct triangle2 *t, vec2 p) {
    float abp = triangle2_signed_area(&(struct triangle2){t->a, t->b, p});
    float bcp = triangle2_signed_area(&(struct triangle2){t->b, t->c, p});
    float cap = triangle2_signed_area(&(struct triangle2){t->c, t->a, p});

    return (abp >= 0 && bcp >= 0 && cap >= 0) || (abp <= 0 && bcp <= 0 && cap <= 0);
}

struct bounding_box
triangle2_get_bounding_box(struct triangle2 *t) {
    return (struct bounding_box){
            min(t->a.x, min(t->b.x, t->c.x)),
            min(t->a.y, min(t->b.y, t->c.y)),
            max(t->a.x, max(t->b.x, t->c.x)),
            max(t->a.y, max(t->b.y, t->c.y)),
    };
}
