//
// Created by tika on 24-9-17.
//

#include <thread>
#include "funcs.h"

/**
 * get greatest or least in the params
 * @param row params
 * @param name function name
 * @return result
 */
Cell *cell_compare(Row *row, const String &name) {
    if (row->empty()) {
        return new Cell();
    }

    var res = new Cell();
    res->type = D_INTEGER;
    for (val &cell: *row) {
        if (check_null(cell) || cell->type == D_BOOL) {
            res->type = D_NULL;
            break;
        } else if (cell->type == D_STRING) {
            res->type = D_STRING;
        } else if (cell->type == D_REAL) {
            res->type = D_REAL;
        }
    }

    if (res->type == D_NULL) {
        return res;
    }

    res->number = row->at(0)->number;
    res->text = row->at(0)->text;
    for (val &cell: *row) {
        var cmp = false;
        if (cell->type == D_STRING) {
            cmp = name == "greatest" ? (cell->text > res->text) : (cell->text < res->text);
            if (cmp) {
                res->text = cell->text;
            }
        } else {
            cmp = name == "greatest" ? (cell->number > res->number) : (cell->number < res->number);
            if (cmp) {
                res->number = cell->number;
            }
        }
    }

    return res;
}

static Cell *tk_coalesce(Row *row) {
    for (val &cell: *row) {
        if (check_null(cell)) {
            continue;
        }
        return new Cell(*cell);
    }

    return new Cell();
}

static Cell *tk_greatest(Row *row) {
    return cell_compare(row, "greatest");
}

static Cell *tk_isnull(Row *row) {
    check_arg_nums(row, "isnull", 1);

    val cell = row->at(0);
    return new Cell(D_BOOL, check_null(cell));
}

static Cell *tk_least(Row *row) {
    return cell_compare(row, "least");
}

static Cell *tk_sleep(Row *row) {
    check_arg_nums(row, "sleep", 1);

    val cell = row->at(0);
    if (!check_null(cell)) {
        if (!check_number(cell)) {
            show_error("sleep: incorrect argument type, must be number");
        }
        val sleep_time = (int) cell->number;
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }

    return new Cell(D_INTEGER, 0);
}

void init_misc_funcs() {
    ADD_NORMAL(coalesce, D_STRING);
    ADD_NORMAL(greatest, D_STRING);
    ADD_NORMAL(isnull, D_BOOL);
    ADD_NORMAL(least, D_STRING);
    ADD_NORMAL(sleep, D_INTEGER);
}
