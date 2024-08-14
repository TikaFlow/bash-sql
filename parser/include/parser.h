//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_PARSER_H
#define BASH_SQL_PARSER_H

#include "util.h"
#include "defs.h"

ASTNode *parse(const String* sql);

#endif //BASH_SQL_PARSER_H
