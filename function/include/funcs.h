//
// Created by tika on 24-9-15.
//

#ifndef BASH_SQL_FUNCS_H
#define BASH_SQL_FUNCS_H

#include "global.h"
#include "defs.h"

extern Map<String, Pair<DataType, Cell *(*)(Row *)>> FUNCTIONS;

void init_funcs();

#endif //BASH_SQL_FUNCS_H
