#ifndef MESH_H
#define MESH_H

#include "array.h"
#include "vec2.h"
#include "vec3.h"

struct vertex {
    int vertex_index, normal_index, texture_index;
};

struct face {
    struct vertex vertices[3];
};

define_array(struct face, face_array);
define_array(vec2, vec2_array);
define_array(vec3, vec3_array);

struct mesh {
    vec3_array_t vertices;
    vec3_array_t normals;
    vec2_array_t textures;

    face_array_t faces;
};

struct mesh *
mesh_load(char *path);

void
mesh_destroy(struct mesh *mesh);

#endif
