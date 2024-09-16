//
// Created by tika on 24-9-16.
//

#include "funcs.h"

static Cell *concat(Row *row) {
    var str = String();

    for (val &cell: *row) {
        str += cell->to_string();
    }

    return new Cell(str);
}

void init_string_funcs() {
    ADD_NORMAL(concat, D_STRING);
}
