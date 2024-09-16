//
// Created by tika on 24-9-16.
//

#include "funcs.h"

static Cell *sum(Row *row) {
    val res = new Cell();
    res->type = D_INTEGER;

    for (val &cell: *row) {
        if (cell->type != D_INTEGER && cell->type != D_REAL) {
            show_error("sum: type error");
        }

        if (cell->text == NONE) {
            continue;
        }

        // any cell is real, then result is real
        if (cell->type == D_REAL) {
            res->type = D_REAL;
        }

        res->number += cell->number;
    }

    return res;
}

void init_agg_funcs() {
    ADD_AGG(sum, D_NUMBER);
}
