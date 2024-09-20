//
// Created by tika on 24-8-13.
//

#include "parser.h"

/**
 * parse sql and return AST tree
 * @param sql the sql to be parsed
 * @return AST tree
 */
Statement parse(Vector<Token *> *tokens) {
    // DEBUG
    // for (val &token: *tokens) {
    //     std::cout << token->to_string() << std::endl;
    // }
    // DEBUG END
    Statement stmt{};

    val start = tokens->at(0);
    switch (start->type) {
        case T_WITH:
        case T_SELECT:
            stmt = parse_read(tokens);
            break;
        default:
            show_error("Unknown SQL statement");
    }

    for (var &token: *tokens) {
        delete token;
    }
    delete tokens;

    return stmt; // make compiler happy
}
