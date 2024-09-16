//
// Created by tika on 24-9-15.
//

#ifndef BASH_SQL_FUNCS_H
#define BASH_SQL_FUNCS_H

#include "global.h"
#include "defs.h"
#include "util.h"

#define ADD_FUNC(name, dtype, ftype) \
FUNCTIONS[#name] = {dtype, ftype, name}

#define ADD_NORMAL(name, dtype) \
ADD_FUNC(name, dtype, F_NORMAL)

#define ADD_AGG(name, dtype) \
ADD_FUNC(name, dtype, F_AGGREGATE)

typedef struct Function Function;

typedef enum {
    F_AGGREGATE, F_NORMAL,
} FuncType;

struct Function {
    DataType dtype;
    FuncType ftype;

    Cell *(*pointer)(Row *);
};

extern Map<String, Function> FUNCTIONS;

void init_math_funcs();

void init_string_funcs();

void init_date_funcs();

void init_logical_funcs();

void init_cast_funcs();

void init_agg_funcs();

void init_misc_funcs();

void init_funcs();

#endif //BASH_SQL_FUNCS_H
