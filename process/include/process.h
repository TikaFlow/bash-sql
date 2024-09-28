//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include "defs.h"
#include "funcs.h"

Result *apply_create(CreateStatement *stmt);

Result *apply_read(SelectStatement *stmt);

Result *process(Statement stmt);

#endif //BASH_SQL_PROCESS_H
