#ifndef CAMERA_H
#define CAMERA_H

#include "state.h"
#include "vec3.h"

struct camera {
    vec3 pos;
    float pitch, yaw;

    // normals
    vec3 normal, right, up;

    int width, height;
    float fov, sensitivity, speed;
};

struct camera *
camera_create(float fov, float sensitivity, float speed);

void
camera_destroy(struct camera *camera);

void
camera_update_position(struct camera *camera, struct keys *pressed, float dt);

void
camera_update_viewport(struct camera *camera, int width, int height);

void
camera_update_orientation(struct camera *camera, float dx, float dy);

#endif
