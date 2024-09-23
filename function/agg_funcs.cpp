//
// Created by tika on 24-9-16.
//

#include "funcs.h"

#define ADD_AGG(name, dtype) \
ADD_FUNC(name, dtype, F_AGGREGATE)

extern Cell *cell_compare(Row *row, const String &name);

extern Cell *concat_aux(Row *row, const String &func_name, const String &sep = "", int start = 0);

/**
 * aux function for avg/count/sum
 * @param row params
 * @param count count of valid cells
 * @return result
 */
static Cell *acs_aux(Row *row, int &count) {
    val res = new Cell();
    res->type = D_INTEGER;
    count = 0;

    for (val &cell: *row) {
        if (check_null(cell) || !check_number(cell)) {
            continue;
        }

        // any cell is real, then result is real
        if (cell->type == D_REAL) {
            res->type = D_REAL;
        }

        res->number += cell->number;
        count++;
    }

    return res;
}

static Cell *tk_avg(Row *row) {
    int count;
    val res = acs_aux(row, count);

    if (!count) {
        res->type = D_NULL;
    } else {
        res->type = D_REAL;
        res->number /= count;
    }

    return res;
}

static Cell *tk_count(Row *row) {
    int count;
    val res = acs_aux(row, count);

    res->type = D_INTEGER;
    res->number = count;
    return res;

}

static Cell *tk_group_concat(Row *row) {
    return concat_aux(row, "group_concat", ",", 0);
}

static Cell *tk_max(Row *row) {
    return cell_compare(row, "greatest");
}

static Cell *tk_min(Row *row) {
    return cell_compare(row, "least");
}

static Cell *tk_sum(Row *row) {
    int count;
    val res = acs_aux(row, count);

    if (!count) {
        res->type = D_NULL;
    }

    return res;
}

void init_agg_funcs() {
    ADD_AGG(avg, D_NUMBER);
    ADD_AGG(count, D_INTEGER);
    ADD_AGG(group_concat, D_STRING);
    ADD_AGG(max, D_STRING);
    ADD_AGG(min, D_STRING);
    ADD_AGG(sum, D_NUMBER);
}
