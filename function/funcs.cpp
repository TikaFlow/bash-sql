//
// Created by tika on 24-9-15.
//

#include "funcs.h"

Map<String, Function> FUNCTIONS;

/**
 * show error when argument number is wrong at function name
 * @param func_name function name
 */
static void wrong_arg_nums(const String &func_name) {
    show_error(func_name + ": wrong number of arguments");
}

void check_arg_nums(Row *row, const String &func_name, int min, int max) {
    if (max == -1) {
        max = min;
    }

    if (row->size() < min || row->size() > max) {
        wrong_arg_nums(func_name);
    }
}

/**
 * check if cell is null
 * @param cell
 * @return
 */
bool check_null(Cell *cell) {
    return cell->type == D_NULL;
}

/**
 * check if the argument cell is a number
 * @param cell param cell
 */
bool check_number(Cell *cell) {
    if (cell->type != D_INTEGER && cell->type != D_REAL) {
        return false;
    }

    return true;
}

/**
 * check if the argument at index is a number
 * @param row params
 * @param func_name function func_name
 * @param index index of the argument, starting from 1
 */
bool check_number(Row *row, const String &func_name, int index) {
    if (row->size() < index) {
        wrong_arg_nums(func_name);
    }

    val cell = row->at(index - 1);
    return check_number(cell);
}

void init_funcs() {
    init_type_funcs();
    init_flow_funcs();
    init_math_funcs();
    init_date_funcs();
    init_string_funcs();
    init_hash_funcs();
    init_agg_funcs();
    init_sys_funcs();
    init_misc_funcs();
}
