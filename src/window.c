#include "window.h"

#include <assert.h>
#include <linux/input-event-codes.h>

#include "alloc.h"
#include "camera.h"
#include "ints.h"
#include "macros.h"
#include "scene.h"
#include "w_desktop_shell.h"
#include "w_keyboard.h"
#include "w_pointer.h"

static void
pointer_handle_enter(struct w_pointer *pointer, struct w_surface *surface, float x, float y) {
    unused(surface), unused(x), unused(y);

    w_pointer_lock(pointer, -1, -1);
    w_pointer_hide_cursor(pointer);
}

static void
pointer_handle_relative_motion(struct w_pointer *pointer, struct w_surface *surface, float dx, float dy,
        float dx_unaccel, float dy_unaccel) {
    unused(surface), unused(dx_unaccel), unused(dy_unaccel);

    struct window *window = pointer->data;
    camera_update_orientation(window->g->camera, dx, dy);
}

static struct w_pointer_listener pointer_listener = {
        .enter = pointer_handle_enter,
        .relative_motion = pointer_handle_relative_motion,
};

static void
keyboard_handle_key(struct w_keyboard *keyboard, struct w_surface *surface, u32 raw, enum wl_keyboard_key_state state) {
    unused(surface);

    struct window *window = keyboard->data;

    switch(raw) {
        case KEY_W: {
            window->g->is_pressed.w = !!state;
            if(window->g->is_pressed.w) {
                window->g->is_pressed.s = false;
            }
            break;
        }
        case KEY_A: {
            window->g->is_pressed.a = !!state;
            if(window->g->is_pressed.a) {
                window->g->is_pressed.d = false;
            }
            break;
        }
        case KEY_S: {
            window->g->is_pressed.s = !!state;
            if(window->g->is_pressed.s) {
                window->g->is_pressed.w = false;
            }
            break;
        }
        case KEY_D: {
            window->g->is_pressed.d = !!state;
            if(window->g->is_pressed.d) {
                window->g->is_pressed.a = false;
            }
            break;
        }
    }
}

static struct w_keyboard_listener keyboard_listener = {
        .key = keyboard_handle_key,
};

static void
render_frame(struct window *window, float dt);

static void
handle_frame(struct w_surface *surface, float dt) {
    unused(dt), unused(surface);

    struct window *window = surface->data;
    render_frame(window, dt);
}

static void
render_frame(struct window *window, float dt) {
    int width = window->g->camera->width;
    int height = window->g->camera->height;

    if(!window->buffer_pool) {
        window->buffer_pool = w_buffer_pool_create(window->g->conn, width, height, 3);
    } else if(width != window->buffer_pool->width || height != window->buffer_pool->height) {
        w_buffer_pool_destroy(window->buffer_pool);
        window->buffer_pool = w_buffer_pool_create(window->g->conn, width, height, 3);
    }

    struct w_buffer *buffer = w_buffer_pool_get_buffer(window->buffer_pool);
    if(!buffer) {
        return;
    }

    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            buffer->data[y * width + x] = 0xff000000;
        }
    }

    camera_update_position(window->g->camera, &window->g->is_pressed, dt);
    scene_render(window->g->scene, window->g->camera, buffer->data);

    w_surface_set_buffer(window->toplevel->surface, buffer);
    w_surface_commit(window->toplevel->surface);

    w_surface_request_frame(window->surface, handle_frame);
}

static void
handle_configure(struct w_toplevel *toplevel) {
    struct window *window = toplevel->data;

    camera_update_viewport(window->g->camera, toplevel->current.width, toplevel->current.height);

    if(!window->mapped) {
        render_frame(window, 0.0f);
    }
}

static void
handle_close(struct w_toplevel *toplevel) {
    struct window *window = toplevel->data;

    w_connection_close(window->g->conn);
}

static struct w_toplevel_listener toplevel_listener = {
        .configure = handle_configure,
        .close = handle_close,
};

struct window *
window_create(struct state *g) {
    struct window *window = alloc(sizeof(*window));
    window->g = g;

    window->pointer = w_pointer_create(window->g->conn);
    assert(window->pointer && w_pointer_can_be_locked(window->pointer) &&
            w_pointer_has_relative_motion(window->pointer));

    window->pointer->data = window;
    w_pointer_add_listener(window->pointer, &pointer_listener);

    window->keyboard = w_keyboard_create(g->conn);
    assert(window->keyboard);

    window->keyboard->data = window;
    w_keyboard_add_listener(window->keyboard, &keyboard_listener);

    window->desktop_shell = w_desktop_shell_create(g->conn);
    assert(window->desktop_shell);

    window->surface = w_surface_create(g->conn);
    window->toplevel = w_toplevel_create(window->desktop_shell, window->surface);

    xdg_toplevel_set_app_id(window->toplevel->xdg_toplevel, "rasterizer");
    xdg_toplevel_set_title(window->toplevel->xdg_toplevel, "rasterizer");
    w_toplevel_set_default_size(window->toplevel, 800, 600);
    w_toplevel_set_min_size(window->toplevel, 800, 600);
    w_toplevel_set_max_size(window->toplevel, 800, 600);

    window->surface->data = window;
    window->toplevel->data = window;
    w_toplevel_add_listener(window->toplevel, &toplevel_listener);

    return window;
}

void
window_destroy(struct window *window) {
    if(window->pointer) {
        w_pointer_destroy(window->pointer);
    }
    if(window->keyboard) {
        w_keyboard_destroy(window->keyboard);
    }
    if(window->buffer_pool) {
        w_buffer_pool_destroy(window->buffer_pool);
    }

    w_surface_destroy(window->surface);
    w_toplevel_destroy(window->toplevel);
    w_desktop_shell_destroy(window->desktop_shell);

    free(window);
}
