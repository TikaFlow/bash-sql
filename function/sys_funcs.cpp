//
// Created by tika on 24-9-19.
//

#include "funcs.h"

static Cell *tk_app_name(Row *row) {
    if (!row->empty()) {
        wrong_arg_nums("app_name");
    }

    return new Cell(APP_NAME);
}

static Cell *tk_author(Row *row) {
    if (!row->empty()) {
        wrong_arg_nums("author");
    }

    return new Cell(AUTHOR);
}

static Cell *tk_version(Row *row) {
    if (!row->empty()) {
        wrong_arg_nums("version");
    }

    return new Cell(VERSION);
}

void init_sys_funcs() {
    ADD_NORMAL(app_name, D_STRING);
    ADD_NORMAL(author, D_STRING);
    ADD_NORMAL(version, D_STRING);
}
