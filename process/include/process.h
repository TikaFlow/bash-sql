//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include <cmath>
#include "getopt_util.h"
#include "defs.h"
#include "funcs.h"

Result *apply(SelectStatement *query, const String &data, int col_count, char d);

#endif //BASH_SQL_PROCESS_H
