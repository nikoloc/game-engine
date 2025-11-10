#ifndef BOX_H
#define BOX_H

struct box {
    int x, y, width, height;
};

struct bounding_box {
    int start_x, start_y, end_x, end_y;
};

#endif
