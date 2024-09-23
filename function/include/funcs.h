//
// Created by tika on 24-9-15.
//

#ifndef BASH_SQL_FUNCS_H
#define BASH_SQL_FUNCS_H

#include "global.h"
#include "defs.h"
#include "util.h"

#define PARAM_MAX 999999

#define ADD_FUNC(name, dtype, ftype) \
FUNCTIONS.insert({#name, {dtype, ftype, tk_##name}})

#define ADD_NORMAL(name, dtype) \
ADD_FUNC(name, dtype, F_NORMAL)

typedef enum {
    F_AGGREGATE, F_NORMAL,
} FuncType;

struct Function {
    DataType dtype;
    FuncType ftype;

    Cell *(*pointer)(Row *);
};

extern Map<String, Function> FUNCTIONS;

void check_arg_nums(Row *row, const String &func_name, int min, int max = -1);

bool check_true(Cell *cell);

bool check_equal(Cell *first, Cell *second);

bool check_null(Cell *cell);

bool check_number(Cell *cell);

bool check_number(Row *row, const String &func_name, int index);

void init_type_funcs();

void init_flow_funcs();

void init_math_funcs();

void init_date_funcs();

void init_string_funcs();

void init_hash_funcs();

void init_agg_funcs();

void init_sys_funcs();

void init_misc_funcs();

void init_funcs();

#endif //BASH_SQL_FUNCS_H
