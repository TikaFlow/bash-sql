//
// Created by tika on 24-9-17.
//

#include "funcs.h"

/**
 * get greatest or least in the params
 * @param row params
 * @param name function name
 * @return result
 */
static Cell *cell_compare(Row *row, const String &name) {
    if (row->empty()) {
        return new Cell(D_INTEGER);
    }

    var res = new Cell();
    res->type = D_INTEGER;
    for (val &cell: *row) {
        if (cell->type == D_BOOL) {
            res->text = NONE;
            break;
        } else if (cell->type == D_STRING) {
            if (cell->text.empty()) {
                res->text = NONE;
                break;
            }
            res->type = D_STRING;
        } else {
            if (cell->text == NONE) {
                res->text = NONE;
                break;
            }
            if (cell->type == D_REAL) {
                res->type = D_REAL;
            }
        }
    }

    if (res->text == NONE) {
        if (res->type == D_STRING) {
            res->text = "";
        }
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

static Cell *tk_greatest(Row *row) {
    return cell_compare(row, "greatest");
}

static Cell *tk_least(Row *row) {
    return cell_compare(row, "least");
}

void init_misc_funcs() {
    ADD_NORMAL(greatest, D_STRING);
    ADD_NORMAL(least, D_STRING);
}
