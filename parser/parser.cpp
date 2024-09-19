//
// Created by tika on 24-8-13.
//

#include "parser.h"

/**
 * parse sql and return AST tree
 * @param sql the sql to be parsed
 * @return AST tree
 */
Vector<SelectStatement *> *parse_sql(const String &sql, int col_count) {
    if (sql.empty()) {
        return null;
    }
    val tokens = lex(sql);
    // DEBUG
    // for (val &token: *tokens) {
    //     std::cout << token->type << " " << token->text << std::endl;
    // }
    // DEBUG END
    return parse(tokens, col_count);
}
