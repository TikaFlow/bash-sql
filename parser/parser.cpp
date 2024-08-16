//
// Created by tika on 24-8-13.
//

#include "parser.h"

ASTNode *parse_sql(const String &sql) {
    if (sql.empty()) {
        return null;
    }
    val tokens = lex(sql);
    // DEBUG
    for (auto &token: *tokens) {
        std::cout << token->type << " " << token->text << std::endl;
    }
    // DEBUG END
    return parse(tokens);
}
