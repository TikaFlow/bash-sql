//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_PARSER_H
#define BASH_SQL_PARSER_H

#include "global.h"
#include "defs.h"

static Token *lex(String sql);

static ASTNode *parse(Token *token);

ASTNode *parse(String sql);

#endif //BASH_SQL_PARSER_H
