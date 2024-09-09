//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_PARSER_H
#define BASH_SQL_PARSER_H

#include "lex.h"
#include "parse.h"

Vector<SelectStatement *> *parse_sql(const String &sql, int col_count);

#endif //BASH_SQL_PARSER_H
