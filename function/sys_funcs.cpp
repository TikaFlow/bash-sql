//
// Created by tika on 24-9-19.
//

#include "funcs.h"

extern void import_data(const String &table);

extern void export_data(const String &table, const String &file, bool with_title, bool with_line_no, char deli);

static Cell *tk_app(Row *row) {
    check_arg_nums(row, "app", 0);

    return new Cell(APP_NAME);
}

static Cell *tk_author(Row *row) {
    check_arg_nums(row, "author", 0);

    return new Cell(AUTHOR);
}

static Cell *tk_export(Row *row) {
    check_arg_nums(row, "export", 2, 5);

    val cell0 = row->at(0);
    val cell1 = row->at(1);
    if (check_null(cell0) || check_null(cell1)
        || cell0->type != D_STRING || cell1->type != D_STRING
        || cell0->text.empty() || cell1->text.empty()) {
        return new Cell();
    }

    val table = cell0->text;
    val file = cell1->text;
    var with_title = false;
    var with_line_no = false;
    var deli = ',';
    if (row->size() > 2) {
        with_title = check_true(row->at(2));
    }
    if (row->size() > 3) {
        with_line_no = check_true(row->at(3));
    }
    if (row->size() > 4 && row->at(4)->type == D_STRING) {
        deli = row->at(4)->text[0];
    }

    export_data(table, file, with_title, with_line_no, deli);
    return new Cell("Export success.");
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

static Cell *tk_import(Row *row) {
    check_arg_nums(row, "import", 2, 5);

    val cell0 = row->at(0);
    val cell1 = row->at(1);
    if (check_null(cell0) || check_null(cell1)
        || cell0->type != D_STRING || cell1->type != D_STRING
        || cell0->text.empty() || cell1->text.empty()) {
        return new Cell();
    }

    val table = cell0->text;
    val file = cell1->text;
    var with_title = false;
    var cols = 0;
    var deli = '\0';
    if (row->size() > 2) {
        with_title = check_true(row->at(2));
    }
    if (row->size() > 3 && check_number(row->at(3))) {
        cols = (int) row->at(3)->number;
        if (cols < 0) {
            cols = 0;
        }
    }
    if (row->size() > 4 && row->at(4)->type == D_STRING) {
        deli = row->at(4)->text[0];
    }

    options->data = read_file_to_string(file);
    options->title = with_title;
    options->columns = cols;
    options->delimiter = deli;

    import_data(table);
    return new Cell("Import success.");
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
    ADD_NORMAL(export, D_STRING);
    ADD_NORMAL(get, D_STRING);
    ADD_NORMAL(import, D_STRING);
    ADD_NORMAL(set, D_BOOL);
    ADD_NORMAL(unset, D_BOOL);
    ADD_NORMAL(version, D_STRING);
}
