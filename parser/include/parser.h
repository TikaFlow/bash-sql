//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_PARSER_H
#define BASH_SQL_PARSER_H

#include "lexer.h"

Statement parse_read(Vector<Token *> *tokens);

Statement parse(Vector<Token *> *tokens);

#endif //BASH_SQL_PARSER_H
