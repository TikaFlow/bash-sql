//
// Created by tika on 24-8-14.
//

#ifndef BASH_SQL_PARSE_H
#define BASH_SQL_PARSE_H

#include "util.h"
#include "defs.h"

ASTNode *parse(Vector<Token *> *tokens);

#endif //BASH_SQL_PARSE_H
