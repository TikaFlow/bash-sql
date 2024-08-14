//
// Created by tika on 24-8-14.
//

#ifndef BASH_SQL_LEX_H
#define BASH_SQL_LEX_H

#include "util.h"
#include "defs.h"

Vector<Token *> *lex(const String *sql);

#endif //BASH_SQL_LEX_H
