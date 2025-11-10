#include "scene.h"

#include "alloc.h"
#include "box.h"
#include "macros.h"
#include "mesh.h"

static void
scene_node_init(struct scene_node *node, struct scene_tree *parent, enum scene_node_type type) {
    node->parent = parent;
    node->type = type;
    node->scale = 1.0f;

    if(parent) {
        scene_node_ptr_array_push(&parent->children, node);
    }
}

struct scene_triangle *
scene_add_triangle(struct scene_tree *parent, struct triangle3 *triangle) {
    struct scene_triangle *scene_triangle = alloc(sizeof(*triangle));
    scene_triangle->triangle = *triangle;

    scene_node_init(&scene_triangle->node, parent, SCENE_NODE_TYPE_TRIANGLE);

    return scene_triangle;
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
    node->pos = pos;
}

void
scene_node_set_rotation(struct scene_node *node, vec3 rot) {
    node->rot = rot;
}

void
scene_node_set_scale(struct scene_node *node, float scale) {
    node->scale = scale;
}

void
scene_node_reparent(struct scene_node *node, struct scene_tree *parent) {
    // find the node in the list
    struct scene_tree *current = node->parent;
    if(current) {
        for(int i = 0; i < current->children.len; i++) {
            if(current->children.data[i] == node && current->children.data[i]->parent) {
                scene_node_ptr_array_remove_fast(&current->children, i);
                break;
            }
        }
    }

    node->parent = parent;

    if(parent) {
        scene_node_ptr_array_push(&parent->children, node);
    }
}

void
scene_node_remove(struct scene_node *node) {
    struct scene_tree *parent = node->parent;
    if(parent) {
        for(int i = 0; i < parent->children.len; i++) {
            if(parent->children.data[i] == node && parent->children.data[i]->parent) {
                scene_node_ptr_array_remove_fast(&parent->children, i);
                break;
            }
        }
    }

    // since this is the first field of an outer struct its okay, tho not ideal
    free(node);
}

static vec2
project_point(struct camera *camera, vec3 point) {
    vec3 rel_pos = vec3_sub(point, camera->pos);

    return (vec2){vec3_dot(rel_pos, camera->right), vec3_dot(rel_pos, camera->up)};
}

static bool
point_on_screen(vec2 point, int width, int height) {
    return point.x >= 0 && point.x < width && point.y >= 0 && point.y < height;
}

static int
camera_to_buffer_coords(struct camera *camera, int x, int y) {
    // invert the y axis since the window starts at top right
    y = camera->height - y - 1;

    return y * camera->width + x;
}

static inline struct triangle3
triangle_apply_things(struct triangle3 *triangle, vec3 pos, vec3 rot, float scale) {
    unused(rot);

    return (struct triangle3){
            vec3_add(vec3_scale(scale, triangle->a), pos),
            vec3_add(vec3_scale(scale, triangle->b), pos),
            vec3_add(vec3_scale(scale, triangle->c), pos),
    };
}

static void
render_triangle(struct triangle3 *triangle, struct camera *camera, u32 *buffer, vec3 pos, vec3 rot, float scale) {
    struct triangle3 t = triangle_apply_things(triangle, pos, rot, scale);
    // printf("a: %f, %f, %f\n", triangle->a.x, triangle->a.y, triangle->a.z);
    // printf("b: %f, %f, %f\n", triangle->b.x, triangle->b.y, triangle->b.z);
    // printf("c: %f, %f, %f\n", triangle->c.x, triangle->c.y, triangle->c.z);

    vec2 a_proj = project_point(camera, t.a);
    // printf("a: %f, %f\n", a_proj.x, a_proj.y);

    vec2 b_proj = project_point(camera, t.b);
    // printf("b: %f, %f\n", b_proj.x, b_proj.y);

    vec2 c_proj = project_point(camera, t.c);
    // printf("c: %f, %f\n", c_proj.x, c_proj.y);

    struct triangle2 proj = {a_proj, b_proj, c_proj};
    // struct triangle2 proj = {{100, 100}, {800, 0}, {1000, 240}};
    struct bounding_box box = triangle2_get_bounding_box(&proj);
    // printf("%d %d %d %d\n", box.start_x, box.end_x, box.start_y, box.end_y);

    for(int x = max(box.start_x, 0); x < min(camera->width, box.end_x); x++) {
        for(int y = max(box.start_y, 0); y < min(box.end_y, camera->height); y++) {
            if(triangle2_contains_point(&proj, (vec2){x, y})) {
                buffer[camera_to_buffer_coords(camera, x, y)] = 0xffff0000;
                // printf("%d\n", camera_to_buffer_coords(camera, x, y));
            }
        }
    }
}

static void
scene_render_iter(struct scene_tree *scene, struct camera *camera, u32 *buffer, vec3 pos, vec3 rot, float scale) {
    // render_triangle(NULL, camera, buffer, (vec3){0}, (vec3){0}, 1.0f);

    for(struct scene_node **node = scene->children.data; node < scene_node_ptr_array_end(&scene->children); node++) {
        vec3 node_pos = vec3_add(pos, (*node)->pos);
        vec3 node_rot = vec3_add(rot, (*node)->rot);
        float node_scale = scale * (*node)->scale;

        // printf("%f, %f, %f\n", node_pos.x, node_pos.y, node_pos.z);
        // printf("%f\n", node_scale);

        switch((*node)->type) {
            case SCENE_NODE_TYPE_MESH: {
                struct scene_mesh *mesh = container_of((*node), struct scene_mesh, node);
                for(struct triangle3 *iter = mesh->mesh->faces.data; iter < triangle3_array_end(&mesh->mesh->faces);
                        iter++) {
                    render_triangle(iter, camera, buffer, node_pos, node_rot, node_scale);
                }
                break;
            }
            default: {
                assert(0 && "todo");
            }
        }
    }

    // todo();
}

void
scene_render(struct scene_tree *scene, struct camera *camera, u32 *buffer) {
    // render it with initial params
    scene_render_iter(scene, camera, buffer, (vec3){0}, (vec3){0}, 1.0f);
}
