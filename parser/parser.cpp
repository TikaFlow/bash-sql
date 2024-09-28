//
// Created by tika on 24-8-13.
//

#include "parser.h"

void Statement::release() {
    switch (this->type) {
        case S_CREATE:
            this->stmt_c->release();
            delete this->stmt_c;
            this->stmt_c = null;
            break;
        case S_SELECT:
            this->stmt_r->release();
            this->stmt_r = null;
            break;
        default:
            break;
    }
}

void CreateStatement::release() {
    this->stmt_r->release();
    delete this->stmt_r;
    this->stmt_r = null;
}

void SelectStatement::release() {
    if (this->with) {
        for (var &w: *this->with) {
            w.stmt->release();
            delete w.stmt;
            w.stmt = null;
        }
        delete this->with;
        this->with = null;
    }

    for (var &s: *this->select) {
        release_node(s.col);
    }
    delete this->select;
    this->select = null;

    release_node(this->from);
    release_node(this->where);

    if (this->group) {
        for (var &g: *this->group) {
            release_node(g);
        }
        delete this->group;
        this->group = null;
    }

    if (this->order) {
        delete this->order;
        this->order = null;
    }

    if (this->limit) {
        delete this->limit;
        this->limit = null;
    }
}

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
    init_parser(tokens);

    val start = tokens->at(0);
    switch (start->type) {
        case T_CREATE:
            stmt = parse_create();
            break;
        case T_WITH:
        case T_SELECT:
            stmt = parse_read();
            break;
        default:
            stmt = {};
            show_error("Unknown SQL statement");
    }

    for (var &token: *tokens) {
        delete token;
    }
    delete tokens;

    return stmt;
}

/**
 * release the memory of AST node
 * @param node the node to release
 */
void release_node(ASTNode *&node) {
    if (!node) {
        return;
    }

    if (node->left) {
        release_node(node->left);
    }
    if (node->right) {
        release_node(node->right);
    }

    delete node;
    node = null;
}
