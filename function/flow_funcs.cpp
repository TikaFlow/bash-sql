//
// Created by tika on 24-9-17.
//

#include "funcs.h"

static Cell *tk_case(Row *row) {
    check_arg_nums(row, "case", 2, PARAM_MAX);

    var dft_idx = 0;
    if (row->size() % 2 == 0) {
        // has a default value
        dft_idx = (int) row->size() - 1;
    }
    val max_idx = dft_idx ? dft_idx : row->size();

    for (var i = 0; i < max_idx; i += 2) {
        val exp = row->at(i);
        val res = row->at(i + 1);

        if (check_true(exp)) {
            return new Cell(*res);
        }
    }

    if (dft_idx) {
        return new Cell(*row->at(dft_idx));
    }

    return new Cell();
}

static Cell *tk_decode(Row *row) {
    check_arg_nums(row, "decode", 3, PARAM_MAX);

    val exp = row->at(0);
    var dft_idx = 0;
    if (row->size() % 2 == 0) {
        // has a default value
        dft_idx = (int) row->size() - 1;
    }
    val max_idx = dft_idx ? dft_idx : row->size();

    for (var i = 1; i < max_idx; i += 2) {
        val value = row->at(i);
        val res = row->at(i + 1);

        if (check_equal(exp, value)) {
            return new Cell(*res);
        }
    }

    if (dft_idx) {
        return new Cell(*row->at(dft_idx));
    }

    return new Cell();
}

static Cell *tk_if(Row *row) {
    check_arg_nums(row, "if", 3);

    val cond = row->at(0);
    val true_val = row->at(1);
    val false_val = row->at(2);

    if (check_true(cond)) {
        return new Cell(*true_val);
    }

    return new Cell(*false_val);
}

static Cell *tk_ifnull(Row *row) {
    check_arg_nums(row, "ifnull", 2);

    val first = row->at(0);
    val second = row->at(1);

    if (check_null(first)) {
        return new Cell(*second);
    }

    return new Cell(*first);
}

static Cell *tk_nullif(Row *row) {
    check_arg_nums(row, "ifnull", 2);

    val first = row->at(0);
    val second = row->at(1);

    if (check_equal(first, second)) {
        return new Cell();
    }

    return new Cell(*first);
}

void init_flow_funcs() {
    ADD_NORMAL(case, D_STRING);
    ADD_NORMAL(decode, D_STRING);
    ADD_NORMAL(if, D_STRING);
    ADD_NORMAL(ifnull, D_STRING);
    ADD_NORMAL(nullif, D_STRING);
}
