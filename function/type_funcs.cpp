//
// Created by tika on 24-9-17.
//

#include "funcs.h"

static Cell *tk_int(Row *row) {
    check_arg_nums(row, "int", 1);

    val cell = row->at(0);
    val res = new Cell(D_INTEGER, 0);
    switch (cell->type) {
        case D_NULL:
            break;
        case D_BOOL:
        case D_INTEGER:
        case D_REAL:
            res->number = (int) cell->number;
            break;
        case D_STRING:
            res->number = (int) strtol(cell->text.c_str(), null, 10);
            break;
        default:
            break; // make compiler happy
    }

    return res;
}

static Cell *tk_double(Row *row) {
    check_arg_nums(row, "double", 1);

    val cell = row->at(0);
    val res = new Cell(D_REAL, 0);
    switch (cell->type) {
        case D_NULL:
            break;
        case D_BOOL:
        case D_INTEGER:
        case D_REAL:
            res->number = cell->number;
            break;
        case D_STRING:
            res->number = strtod(cell->text.c_str(), null);
            break;
        default:
            break; // make compiler happy
    }

    return res;

}

static Cell *tk_string(Row *row) {
    check_arg_nums(row, "string", 1);

    val cell = row->at(0);
    return new Cell(cell->to_string());

}

void init_type_funcs() {
    ADD_NORMAL(int, D_INTEGER);
    ADD_NORMAL(double, D_REAL);
    ADD_NORMAL(string, D_STRING);
}
