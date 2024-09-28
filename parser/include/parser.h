//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_PARSER_H
#define BASH_SQL_PARSER_H

#include "lexer.h"

void init_parser(Vector<Token *> *tokens);

void release_node(ASTNode *&node);

Statement parse_create();

Statement parse_read();

Statement parse(Vector<Token *> *tokens);

#endif //BASH_SQL_PARSER_H
