#include "scene.h"

#include "alloc.h"
#include "box.h"
#include "macros.h"
#include "mesh.h"
#include "triangle.h"

static inline void
transform_default(struct transform *dest) {
    dest->pos = (vec3){0};
    dest->rot = mat3_identity();
    dest->scale = 1.0f;
}

static void
scene_node_init(struct scene_node *node, struct scene_tree *parent, enum scene_node_type type) {
    node->parent = parent;
    node->type = type;
    transform_default(&node->transform);

    if(parent) {
        scene_node_ptr_array_push(&parent->children, node);
    }
}

struct scene_polygon *
scene_add_polygon(struct scene_tree *parent, int len, vec3 vertices[len]) {
    struct scene_polygon *scene_polygon = alloc(sizeof(*scene_polygon));
    todo("add polygon to triangles conversion");

    scene_node_init(&scene_polygon->node, parent, SCENE_NODE_TYPE_POLYGON);

    return scene_polygon;
}

struct scene_mesh *
scene_add_mesh(struct scene_tree *parent, struct mesh *mesh) {
    struct scene_mesh *scene_mesh = alloc(sizeof(*scene_mesh));
    scene_mesh->mesh = mesh;

    scene_node_init(&scene_mesh->node, parent, SCENE_NODE_TYPE_MESH);

    return scene_mesh;
}

struct scene_tree *
scene_add_tree(struct scene_tree *parent) {
    struct scene_tree *scene_tree = alloc(sizeof(*scene_tree));

    scene_node_init(&scene_tree->node, parent, SCENE_NODE_TYPE_TREE);

    return scene_tree;
}

void
scene_node_set_position(struct scene_node *node, vec3 pos) {
    node->transform.pos = pos;
}

static inline mat3
get_rotation_matrix(vec3 rot) {
    return mat3_mul(mat3_mul(mat3_rotation_z(rot.z), mat3_rotation_x(rot.x)), mat3_rotation_y(rot.y));
}

void
scene_node_set_rotation(struct scene_node *node, vec3 rot) {
    node->transform.rot = get_rotation_matrix(rot);
}

void
scene_node_set_scale(struct scene_node *node, float scale) {
    node->transform.scale = scale;
}

static void
remove_node_from_parents_children(struct scene_node *node) {
    for(int i = 0; i < node->parent->children.len; i++) {
        if(node->parent->children.data[i] == node) {
            scene_node_ptr_array_remove_fast(&node->parent->children, i);
            return;
        }
    }
}

void
scene_node_reparent(struct scene_node *node, struct scene_tree *parent) {
    if(node->parent) {
        remove_node_from_parents_children(node);
    }

    node->parent = parent;
    if(parent) {
        scene_node_ptr_array_push(&parent->children, node);
    }
}

static void
scene_node_remove_iter(struct scene_node *node) {
    switch(node->type) {
        case SCENE_NODE_TYPE_TREE: {
            struct scene_tree *tree = container_of(node, struct scene_tree, node);
            for(struct scene_node **iter = tree->children.data; iter < scene_node_ptr_array_end(&tree->children);
                    iter++) {
                scene_node_remove_iter(*iter);
            }
            scene_node_ptr_array_deinit(&tree->children);
            free(tree);
            break;
        }
        case SCENE_NODE_TYPE_POLYGON: {
            struct scene_polygon *polygon = container_of(node, struct scene_polygon, node);
            todo("free the polygon struct fields");
            free(polygon);
            break;
        }
        case SCENE_NODE_TYPE_MESH: {
            struct scene_mesh *mesh = container_of(node, struct scene_mesh, node);
            free(mesh);
            break;
        }
    }
}

void
scene_node_remove(struct scene_node *node) {
    if(node->parent) {
        remove_node_from_parents_children(node);
    }

    // this will remove the object nodes, and iteratively remove the tree's children and childrens children etc
    scene_node_remove_iter(node);
}

static bool
project_point(struct camera *camera, vec3 point, vec2 *dest) {
    vec3 rel = vec3_sub(point, camera->pos);
    float depth = vec3_dot(rel, camera->normal);

    if(depth <= 0.0f) {
        return false;
    }

    float f = 1.0f / tanf(camera->fov * 0.5f);

    float x = vec3_dot(rel, camera->right) / depth;
    float y = vec3_dot(rel, camera->up) / depth;

    x *= f / ((float)camera->width / camera->height);
    y *= f;

    dest->x = (x + 1.0f) * 0.5f * camera->width;
    dest->y = (1.0f - (y + 1.0f) * 0.5f) * camera->height;

    return true;
}

static inline int
camera_to_buffer_coords(struct camera *camera, int x, int y) {
    return y * camera->width + x;
}

struct face_render_data {
    struct vertex_render_data {
        vec3 vertex;
        bool has_normal;
        vec3 normal;
        bool has_texture;
        vec2 texture;
    } vertices[3];
};

static void
face_get_render_data(struct mesh *mesh, int index, struct face_render_data *dest) {
    *dest = (struct face_render_data){0};

    struct face *face = &mesh->faces.data[index];
    for(int i = 0; i < 3; i++) {
        int j = face->vertices[i].vertex_index;
        dest->vertices[i].vertex = mesh->vertices.data[j];

        j = face->vertices[i].normal_index;
        if(j >= 0) {
            dest->vertices[i].normal = mesh->normals.data[j];
            dest->vertices[i].has_normal = true;
        }

        j = face->vertices[i].texture_index;
        if(j >= 0) {
            dest->vertices[i].texture = mesh->textures.data[j];
            dest->vertices[i].has_texture = true;
        }
    }
}

static inline void
face_transform(struct face_render_data *face, struct transform *transform) {
    vec3 pos = transform->pos;
    float scale = transform->scale;
    mat3 rot = transform->rot;

    face->vertices[0].vertex = mat3_mul_vec3(rot, face->vertices[0].vertex);
    face->vertices[1].vertex = mat3_mul_vec3(rot, face->vertices[1].vertex);
    face->vertices[2].vertex = mat3_mul_vec3(rot, face->vertices[2].vertex);

    // also rotate the normals
    face->vertices[0].normal = mat3_mul_vec3(rot, face->vertices[0].normal);
    face->vertices[1].normal = mat3_mul_vec3(rot, face->vertices[1].normal);
    face->vertices[2].normal = mat3_mul_vec3(rot, face->vertices[2].normal);

    // and then scale and translate the positions
    face->vertices[0].vertex = vec3_add(vec3_scale(scale, face->vertices[0].vertex), pos);
    face->vertices[1].vertex = vec3_add(vec3_scale(scale, face->vertices[1].vertex), pos);
    face->vertices[2].vertex = vec3_add(vec3_scale(scale, face->vertices[2].vertex), pos);
}

static void
render_face(struct face_render_data *face, struct camera *camera, struct transform *transform, u32 *buffer) {
    face_transform(face, transform);

    struct triangle2 proj;
    for(int i = 0; i < 3; i++) {
        if(!project_point(camera, face->vertices[i].vertex, &proj.vertices[i])) {
            return;
        }
    }

    struct bounding_box box = triangle2_get_bounding_box(&proj);

    for(int x = max(box.start_x, 0); x < min(camera->width, box.end_x); x++) {
        for(int y = max(box.start_y, 0); y < min(box.end_y, camera->height); y++) {
            if(triangle2_contains_point(&proj, (vec2){x, y})) {
                buffer[camera_to_buffer_coords(camera, x, y)] = 0xff00ffff;
            }
        }
    }
}

static void
transform_add(struct transform *dest, struct transform *other) {
    dest->pos = vec3_add(other->pos, dest->pos);
    dest->rot = mat3_mul(other->rot, dest->rot);
    dest->scale *= other->scale;
}

static void
scene_render_iter(struct scene_tree *scene, struct camera *camera, struct transform *transform, u32 *buffer) {
    for(struct scene_node **node = scene->children.data; node < scene_node_ptr_array_end(&scene->children); node++) {
        struct transform current_transform = (*node)->transform;
        transform_add(&current_transform, transform);

        switch((*node)->type) {
            case SCENE_NODE_TYPE_MESH: {
                struct scene_mesh *mesh = container_of((*node), struct scene_mesh, node);

                struct face_render_data data;
                for(int i = 0; i < mesh->mesh->faces.len; i++) {
                    face_get_render_data(mesh->mesh, i, &data);
                    render_face(&data, camera, &current_transform, buffer);
                }
                break;
            }
            default: {
                todo();
            }
        }
    }
}

void
scene_render(struct scene_tree *scene, struct camera *camera, u32 *buffer) {
    // render it with initial params
    struct transform transform;
    transform_default(&transform);

    scene_render_iter(scene, camera, &transform, buffer);
}
