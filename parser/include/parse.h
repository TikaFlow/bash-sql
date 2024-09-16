//
// Created by tika on 24-8-14.
//

#ifndef BASH_SQL_PARSE_H
#define BASH_SQL_PARSE_H

#include "util.h"
#include "funcs.h"

Vector<SelectStatement *> *parse(Vector<Token *> *tokens, int col_count);

#endif //BASH_SQL_PARSE_H
