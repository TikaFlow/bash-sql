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

static Cell *tk_version(Row *row) {
    check_arg_nums(row, "version", 0);

    return new Cell(VERSION);
}

void init_sys_funcs() {
    ADD_NORMAL(app, D_STRING);
    ADD_NORMAL(author, D_STRING);
    ADD_NORMAL(version, D_STRING);
}
