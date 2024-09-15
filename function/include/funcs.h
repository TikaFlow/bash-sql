//
// Created by tika on 24-9-15.
//

#ifndef BASH_SQL_FUNCS_H
#define BASH_SQL_FUNCS_H

#include "global.h"
#include "defs.h"

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

void init_funcs();

#endif //BASH_SQL_FUNCS_H
