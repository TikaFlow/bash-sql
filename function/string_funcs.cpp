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

static Cell *tk_hex(Row *row) {
    if (row->size() != 1) {
        show_error("hex: wrong number of arguments");
    }

    val cell = row->at(0);
    if (cell->type != D_INTEGER && cell->type != D_STRING) {
        show_error("hex: argument must be a integer or string");
    }

    std::ostringstream oss;
    if (cell->type == D_INTEGER) {
        oss << std::uppercase << std::hex << (int) cell->number;
    } else {
        for (char c: cell->text) {
            oss << std::uppercase << std::hex << (int) c;
        }
    }

    return new Cell(oss.str());
}

void init_string_funcs() {
    ADD_NORMAL(concat, D_STRING);
    ADD_NORMAL(hex, D_STRING);
}
