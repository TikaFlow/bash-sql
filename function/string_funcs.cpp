//
// Created by tika on 24-9-16.
//

#include "funcs.h"

static Cell *tk_concat(Row *row) {
    var str = String();

    for (val &cell: *row) {
        if ((cell->type == D_STRING && cell->text.empty())
            || (cell->type != D_STRING && cell->text == NONE)) {
            return new Cell(D_STRING);
        }
        str += cell->to_string();
    }

    return new Cell(str);
}

void init_string_funcs() {
    ADD_NORMAL(concat, D_STRING);
}
