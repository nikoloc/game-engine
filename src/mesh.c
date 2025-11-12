#include "mesh.h"

#include "alloc.h"
#include "reader.h"

static bool
add_vertex(struct mesh *mesh, string_array_t *parts) {
    if(parts->len != 4) {
        return false;
    }

    vec3_array_push(&mesh->vertices,
            (vec3){
                    atof(string_c_string_view(&parts->data[1])),
                    atof(string_c_string_view(&parts->data[2])),
                    atof(string_c_string_view(&parts->data[3])),
            });

    return true;
}

static bool
add_normal(struct mesh *mesh, string_array_t *parts) {
    if(parts->len != 4) {
        return false;
    }

    vec3_array_push(&mesh->normals,
            (vec3){
                    atof(string_c_string_view(&parts->data[1])),
                    atof(string_c_string_view(&parts->data[2])),
                    atof(string_c_string_view(&parts->data[3])),
            });

    return true;
}

static bool
add_texture(struct mesh *mesh, string_array_t *parts) {
    if(parts->len != 3) {
        return false;
    }

    vec2_array_push(&mesh->textures,
            (vec2){
                    atof(string_c_string_view(&parts->data[1])),
                    atof(string_c_string_view(&parts->data[2])),
            });

    return true;
}

static bool
parse_face_vertex(string_t *s, struct vertex *dest) {
    string_array_t parts = {0};
    string_split(s, '/', false, &parts);

    bool ret;
    if(parts.len == 1) {
        dest->vertex_index = atoi(string_c_string_view(&parts.data[0])) - 1;
        dest->texture_index = -1;
        dest->normal_index = -1;

        ret = true;
    } else if(parts.len == 3) {
        dest->vertex_index = atoi(string_c_string_view(&parts.data[0])) - 1;
        dest->texture_index = atoi(string_c_string_view(&parts.data[1])) - 1;
        dest->normal_index = atoi(string_c_string_view(&parts.data[2])) - 1;

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
is_valid_vertex(struct mesh *mesh, struct vertex *vertex) {
    return vertex->vertex_index >= 0 && vertex->vertex_index < mesh->vertices.len &&
            vertex->normal_index < mesh->normals.len && vertex->texture_index < mesh->textures.len;
}

static bool
add_face(struct mesh *mesh, string_array_t *parts) {
    if(parts->len < 4) {
        return false;
    }

    struct vertex first = {-1, -1, -1}, last = {-1, -1, -1}, cur;
    for(int i = 1; i < parts->len; i++) {
        if(!parse_face_vertex(&parts->data[i], &cur) || !is_valid_vertex(mesh, &cur)) {
            return false;
        }

        if(first.vertex_index == -1) {
            first = cur;
        } else if(last.vertex_index == -1) {
            last = cur;
        } else {
            face_array_push(&mesh->faces, (struct face){first, last, cur});
            last = cur;
        }
    }

    return true;
}

struct mesh *
mesh_load(char *path) {
    struct reader *r = reader_create(path);
    if(!r) {
        return NULL;
    }

    struct mesh *mesh = alloc(sizeof(*mesh));

    string_t line = {0};
    string_array_t parts = {0};
    while(reader_read_line(r, &line)) {
        string_split(&line, ' ', true, &parts);

        string_t *key = &parts.data[0];
        if(string_equal_c_string(key, "v")) {
            if(!add_vertex(mesh, &parts)) {
                goto err;
            }
        } else if(string_equal_c_string(key, "vn")) {
            if(!add_normal(mesh, &parts)) {
                goto err;
            }
        } else if(string_equal_c_string(key, "vt")) {
            if(!add_texture(mesh, &parts)) {
                goto err;
            }
        } else if(string_equal_c_string(key, "f")) {
            if(!add_face(mesh, &parts)) {
                goto err;
            }
        }

        // free the substrings
        for(struct string *iter = parts.data; iter < string_array_end(&parts); iter++) {
            string_deinit(iter);
        }
    }

    string_array_deinit(&parts);
    string_deinit(&line);
    reader_destroy(r);

    return mesh;

err:
    for(struct string *iter = parts.data; iter < string_array_end(&parts); iter++) {
        string_deinit(iter);
    }

    string_array_deinit(&parts);
    string_deinit(&line);
    reader_destroy(r);
    mesh_destroy(mesh);

    return NULL;
}

void
mesh_destroy(struct mesh *mesh) {
    vec3_array_deinit(&mesh->vertices);
    vec3_array_deinit(&mesh->normals);
    vec2_array_deinit(&mesh->textures);

    face_array_deinit(&mesh->faces);

    free(mesh);
}
