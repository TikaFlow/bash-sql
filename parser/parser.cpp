//
// Created by tika on 24-8-13.
//

#include "parser.h"

ASTNode *parse(const String *sql) {
    if (!sql || sql->empty()) {
        return null;
    }
    val tokens = lex(sql);
    return parse(tokens);
}
