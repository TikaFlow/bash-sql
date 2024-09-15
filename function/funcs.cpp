//
// Created by tika on 24-9-15.
//

#include "funcs.h"

#define ADD_FUNC(name, type) \
FUNCTIONS[#name] = {(type), (name)}

Map<String, Pair<DataType, Cell *(*)(Row *)>> FUNCTIONS;

static Cell *concat(Row *row) {
    var str = String();

    for (val &cell: *row) {
        str += cell->to_string();
    }

    return new Cell(str);
}

void init_funcs() {
    ADD_FUNC(concat, D_STRING);
}
