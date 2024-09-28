//
// Created by tika on 24-9-19.
//

#include "funcs.h"

static Cell *tk_app(Row *row) {
    check_arg_nums(row, "app", 0);

    return new Cell(APP_NAME);
}

static Cell *tk_author(Row *row) {
    check_arg_nums(row, "author", 0);

    return new Cell(AUTHOR);
}

static Cell *tk_get(Row *row) {
    check_arg_nums(row, "get", 1);

    val cell = row->at(0);
    if (check_null(cell) || cell->type != D_STRING) {
        return new Cell();
    }

    val env = getenv(cell->text.c_str());
    return new Cell(env ? String(env) : "");
}

static Cell *tk_set(Row *row) {
    check_arg_nums(row, "set", 2);

    val cell = row->at(0);
    if (check_null(cell) || cell->type != D_STRING || cell->text.empty()) {
        return new Cell();
    }

    val name = cell->text;
    val value = row->at(1)->to_string();

    return new Cell(D_BOOL, setenv(name.c_str(), value.c_str(), 1) + 1);
}

static Cell *tk_unset(Row *row) {
    check_arg_nums(row, "unset", 1);

    val cell = row->at(0);
    if (check_null(cell) || cell->type != D_STRING) {
        return new Cell();
    }

    return new Cell(D_BOOL, unsetenv(cell->text.c_str()) + 1);
}

static Cell *tk_version(Row *row) {
    check_arg_nums(row, "version", 0);

    return new Cell(VERSION);
}

void init_sys_funcs() {
    ADD_NORMAL(app, D_STRING);
    ADD_NORMAL(author, D_STRING);
    ADD_NORMAL(get, D_STRING);
    ADD_NORMAL(set, D_BOOL);
    ADD_NORMAL(unset, D_BOOL);
    ADD_NORMAL(version, D_STRING);
}
