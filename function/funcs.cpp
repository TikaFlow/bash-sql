//
// Created by tika on 24-9-15.
//

#include "funcs.h"

#define ADD_FUNC(name, dtype, ftype) \
FUNCTIONS[#name] = {dtype, ftype, name}

Map<String, Function> FUNCTIONS;

static Cell *concat(Row *row) {
    var str = String();

    for (val &cell: *row) {
        str += cell->to_string();
    }

    return new Cell(str);
}

static Cell *sum(Row *row) {
    val res = new Cell();
    res->type = row->at(0)->type;

    for (val &cell: *row) {
        res->number += cell->number;
    }

    return res;
}

void init_funcs() {
    ADD_FUNC(concat, D_STRING, F_NORMAL);
    ADD_FUNC(sum, D_NUMBER, F_AGGREGATE);
}
