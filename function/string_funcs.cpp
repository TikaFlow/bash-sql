//
// Created by tika on 24-9-16.
//

#include "funcs.h"

#define PARAM_MAX 999999

Cell *concat_aux(Row *row, const String &func_name, const String &sep = "", int start = 0);

/**
 * aux function for concat/concat_ws/group_concat
 * @param row params
 * @param func_name who called this function
 * @param sep separator
 * @param start start index
 * @return concatenated string cell
 */
Cell *concat_aux(Row *row, const String &func_name, const String &sep, int start) {
    check_arg_nums(row, func_name, start + 1, PARAM_MAX);

    var str = String();
    for (int i = start; i < row->size(); ++i) {
        val cell = row->at(i);
        if (check_null(cell)) {
            return new Cell();
        }
        str += cell->to_string();

        if (i == row->size() - 1) {
            break;
        }
        str += sep;
    }

    return new Cell(str);
}

static Cell *tk_concat(Row *row) {
    return concat_aux(row, "concat");
}

static Cell *tk_concat_ws(Row *row) {
    val sep = row->at(0);
    if (sep->type == D_NULL) {
        return new Cell();
    }

    return concat_aux(row, "concat_ws", sep->to_string(), 1);
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
    ADD_NORMAL(concat_ws, D_STRING);
    ADD_NORMAL(hex, D_STRING);
}
