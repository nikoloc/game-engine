#ifndef SCENE_H
#define SCENE_H

#include "array.h"
#include "camera.h"
#include "ints.h"
#include "triangle.h"
#include "vec3.h"

enum scene_node_type {
    SCENE_NODE_TYPE_TREE,
    SCENE_NODE_TYPE_TRIANGLE,
    SCENE_NODE_TYPE_MESH,
};

struct scene_node {
    // use this fiels and `container_of()` macro to retrive the appropriate structure
    enum scene_node_type type;
    struct scene_tree *parent;

    vec3 pos;
    vec3 rot;
    float scale;
};

// is is important to keep the node as the first field of these
struct scene_triangle {
    struct scene_node node;

    struct triangle3 triangle;
};

struct scene_mesh {
    struct scene_node node;

    struct mesh *mesh;
};

define_array(struct scene_node *, scene_node_ptr_array);

struct scene_tree {
    struct scene_node node;

    scene_node_ptr_array_t children;
};

struct scene_triangle *
scene_add_triangle(struct scene_tree *parent, struct triangle3 *triangle);

struct scene_mesh *
scene_add_mesh(struct scene_tree *parent, struct mesh *mesh);

struct scene_tree *
scene_add_tree(struct scene_tree *parent);

void
scene_node_set_position(struct scene_node *node, vec3 pos);

void
scene_node_set_rotation(struct scene_node *node, vec3 rot);

void
scene_node_set_scale(struct scene_node *node, float scale);

void
scene_node_reparent(struct scene_node *node, struct scene_tree *parent);

void
scene_node_remove(struct scene_node *node);

void
scene_render(struct scene_tree *scene, struct camera *camera, u32 *buffer);

#endif
