#ifndef MESH_H
#define MESH_H

#include "array.h"
#include "triangle.h"

define_array(struct triangle3, triangle3_array);

struct mesh {
    triangle3_array_t faces;
    // materials etc
};

struct mesh *
mesh_load(char *path);

void
mesh_destroy(struct mesh *mesh);

#endif
