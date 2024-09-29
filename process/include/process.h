//
// Created by tika on 5/2/24.
//

#ifndef BASH_SQL_PROCESS_H
#define BASH_SQL_PROCESS_H

#include "defs.h"
#include "funcs.h"

Result *apply_create(const String &name, SelectStatement *stmt);

Result *apply_read(SelectStatement *stmt);

Result *apply_drop(const String &name);

Result *apply_delete(DeleteStatement *stmt);

Result *apply_describe(const String &name);

Result *apply_show();

Result *apply_bang(int n);

Result *apply_history();

Result *process(const Statement &stmt);

#endif //BASH_SQL_PROCESS_H
