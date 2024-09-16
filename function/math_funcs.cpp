//
// Created by tika on 24-9-16.
//

#include <cmath>
#include "funcs.h"

static Cell *mod(Row *row) {
    if (row->size() != 2) {
        show_error("mod: wrong number of arguments");
    }

    val left = row->at(0);
    val right = row->at(1);

    if ((left->type != D_INTEGER && left->type != D_REAL)
        || (right->type != D_INTEGER && right->type != D_REAL)) {
        show_error("mod: arguments must be numbers");
    }

    if (left->text == NONE || right->text == NONE) {
        return null;
    }

    if (right->number == 0) {
        show_error("mod: division by zero");
    }

    val cell = new Cell();
    cell->type = left->type;
    cell->type = right->type == D_REAL ? D_REAL : cell->type;
    cell->number = fmod(left->number, right->number);

    return cell;
}

void init_math_funcs() {
    ADD_NORMAL(mod, D_NUMBER);
}
