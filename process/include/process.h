//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include "defs.h"
#include "funcs.h"

Result *apply_read(SelectStatement *query, const String &data, int col_count, char d);

Result *process(Statement stmt);

#endif //BASH_SQL_PROCESS_H
