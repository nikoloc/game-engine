#include "triangle.h"

#include <stdbool.h>

#include "box.h"
#include "macros.h"

float
triangle2_signed_area(struct triangle2 *triangle) {
    vec2 ac = vec2_sub(triangle->vertices[0], triangle->vertices[2]);
    vec2 bc = vec2_sub(triangle->vertices[1], triangle->vertices[2]);

    return (ac.x * bc.y - ac.y * bc.x) / 2.0f;
}

bool
triangle2_contains_point(struct triangle2 *triangle, vec2 p) {
    float abp = triangle2_signed_area(&(struct triangle2){triangle->vertices[0], triangle->vertices[1], p});
    float bcp = triangle2_signed_area(&(struct triangle2){triangle->vertices[1], triangle->vertices[2], p});
    float cap = triangle2_signed_area(&(struct triangle2){triangle->vertices[2], triangle->vertices[0], p});

    return (abp >= 0 && bcp >= 0 && cap >= 0) || (abp <= 0 && bcp <= 0 && cap <= 0);
}

struct bounding_box
triangle2_get_bounding_box(struct triangle2 *triangle) {
    return (struct bounding_box){
            min(triangle->vertices[0].x, min(triangle->vertices[1].x, triangle->vertices[2].x)),
            min(triangle->vertices[0].y, min(triangle->vertices[1].y, triangle->vertices[2].y)),
            max(triangle->vertices[0].x, max(triangle->vertices[1].x, triangle->vertices[2].x)),
            max(triangle->vertices[0].y, max(triangle->vertices[1].y, triangle->vertices[2].y)),
    };
}
