#include <assert.h>
#include <w_connection.h>
#include <w_desktop_shell.h>
#include <w_keyboard.h>
#include <w_pointer.h>

#include "camera.h"
#include "mesh.h"
#include "scene.h"
#include "state.h"
#include "window.h"

int
main(void) {
    struct state g = {0};

    g.conn = w_connection_create(NULL);
    assert(g.conn);

    g.window = window_create(&g);
    g.camera = camera_create(M_PI_2, 1.0f / 4096.0f, 4.0f);
    g.camera->pos.z = -200.0f;
    g.scene = scene_add_tree(NULL);

    struct mesh *cube = mesh_load("assets/meshes/cube.obj");
    assert(cube);

    struct scene_mesh *scene_cube = scene_add_mesh(g.scene, cube);
    scene_node_set_scale(&scene_cube->node, 100.0f);

    w_connection_listen(g.conn);

    scene_node_remove(&scene_cube->node);
    mesh_destroy(cube);
    scene_node_remove(&g.scene->node);
    camera_destroy(g.camera);
    window_destroy(g.window);
    w_connection_destroy(g.conn);

    return 0;
}
