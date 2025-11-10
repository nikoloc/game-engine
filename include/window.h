#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <stdbool.h>

#include "state.h"

struct window {
    struct w_pointer *pointer;
    struct w_keyboard *keyboard;
    struct w_buffer_pool *buffer_pool;

    struct w_surface *surface;
    struct w_desktop_shell *desktop_shell;
    struct w_toplevel *toplevel;

    struct state *g;

    bool mapped;
};

struct window *
window_create(struct state *g);

void
window_destroy(struct window *window);

#endif
