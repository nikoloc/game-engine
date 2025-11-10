#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

struct keys {
    bool w, a, s, d;
};

struct state {
    struct w_connection *conn;
    struct camera *camera;
    struct scene_tree *scene;

    struct window *window;

    struct keys is_pressed;
};

#endif
