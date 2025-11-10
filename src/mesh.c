#include "mesh.h"

#include <stdio.h>

#include "alloc.h"
#include "macros.h"
#include "reader.h"

define_array(vec3, vec3_array);

static bool
add_vertex(vec3_array_t *vertecies, string_array_t *parts) {
    if(parts->len != 4) {
        return false;
    }

    vec3_array_push(vertecies,
            (vec3){
                    atof(string_c_string_view(&parts->data[1])),
                    atof(string_c_string_view(&parts->data[2])),
                    atof(string_c_string_view(&parts->data[3])),
            });

    return true;
}

static bool
parse_face_vertex(string_t *s, int *vertex, int *texture, int *normal) {
    string_array_t parts = {0};
    string_split(s, '/', false, &parts);

    bool ret;
    if(parts.len == 1) {
        *vertex = atoi(string_c_string_view(&parts.data[0])) - 1;
        *texture = -1;
        *normal = -1;

        ret = true;
    } else if(parts.len == 3) {
        *vertex = atoi(string_c_string_view(&parts.data[0])) - 1;
        *texture = atoi(string_c_string_view(&parts.data[1])) - 1;
        *normal = atoi(string_c_string_view(&parts.data[2])) - 1;

        ret = true;
    } else {
        ret = false;
    }

    for(struct string *iter = parts.data; iter < string_array_end(&parts); iter++) {
        string_deinit(iter);
    }
    string_array_deinit(&parts);

    return ret;
}

static bool
add_face(struct triangle3_array *faces, struct vec3_array *vertecies, string_array_t *parts) {
    if(parts->len < 4) {
        printf("parts\n");
        return false;
    }

    int vertex, texture, normal;
    struct triangle3 face;

    if(!parse_face_vertex(&parts->data[1], &vertex, &texture, &normal) || vertex >= vertecies->len) {
        return false;
    }
    face.a = vertecies->data[vertex];

    if(!parse_face_vertex(&parts->data[2], &vertex, &texture, &normal) || vertex >= vertecies->len) {
        return false;
    }
    face.b = vertecies->data[vertex];

    if(!parse_face_vertex(&parts->data[3], &vertex, &texture, &normal) || vertex >= vertecies->len) {
        return false;
    }
    face.c = vertecies->data[vertex];

    triangle3_array_push(faces, face);

    return true;
}

struct mesh *
mesh_load(char *path) {
    struct reader *r = reader_create(path);
    if(!r) {
        return NULL;
    }

    struct mesh *mesh = alloc(sizeof(*mesh));
    struct vec3_array vertecies = {0}, normals = {0};

    string_t line = {0};
    string_array_t parts = {0};
    while(reader_read_line(r, &line)) {
        string_split(&line, ' ', true, &parts);

        string_t *key = &parts.data[0];
        if(string_equal_c_string(key, "v")) {
            if(!add_vertex(&vertecies, &parts)) {
                goto err;
            }
        } else if(string_equal_c_string(key, "vt")) {
        } else if(string_equal_c_string(key, "vn")) {
        } else if(string_equal_c_string(key, "f")) {
            if(!add_face(&mesh->faces, &vertecies, &parts)) {
                goto err;
            }
        }

        // free the substrings
        for(struct string *iter = parts.data; iter < string_array_end(&parts); iter++) {
            string_deinit(iter);
        }
    }

    // int i = 0;
    // for(struct triangle3 *iter = mesh->faces.data; iter < triangle3_array_end(&mesh->faces); iter++) {
    //     printf("A: %f, %f, %f\n", iter->a.x, iter->a.y, iter->a.z);
    //     printf("B: %f, %f, %f\n", iter->b.x, iter->b.y, iter->b.z);
    //     printf("C: %f, %f, %f\n\n", iter->c.x, iter->c.y, iter->c.z);
    //
    //     if(i > 100)
    //         break;
    //     i++;
    // }
    // printf("total: %d\n", mesh->faces.len);

    string_array_deinit(&parts);
    string_deinit(&line);
    vec3_array_deinit(&vertecies);
    vec3_array_deinit(&normals);
    reader_destroy(r);

    return mesh;

err:
    for(struct string *iter = parts.data; iter < string_array_end(&parts); iter++) {
        string_deinit(iter);
    }
    string_array_deinit(&parts);
    string_deinit(&line);
    vec3_array_deinit(&vertecies);
    vec3_array_deinit(&normals);
    reader_destroy(r);
    triangle3_array_deinit(&mesh->faces);
    free(mesh);
    return NULL;
}

void
mesh_destroy(struct mesh *mesh) {
    triangle3_array_deinit(&mesh->faces);
    free(mesh);
}
