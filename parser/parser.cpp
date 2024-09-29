//
// Created by tika on 24-8-13.
//

#include "parser.h"

void Statement::release() {
    switch (this->type) {
        case S_CREATE:
        case S_SELECT:
            this->stmt_select->release();
            delete this->stmt_select;
            this->stmt_select = null;
            break;
            // case S_INSERT:
            //     this->stmt_insert->release();
            //     delete this->stmt_insert;
            //     this->stmt_insert = null;
            //     break;
        case S_UPDATE:
            this->stmt_update->release();
            delete this->stmt_update;
            this->stmt_update = null;
            break;
        case S_DELETE:
            this->stmt_delete->release();
            delete this->stmt_delete;
            this->stmt_delete = null;
            break;
        default:
            break;
    }
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

void DeleteStatement::release() {
    release_node(this->from);
    release_node(this->where);
}

void UpdateStatement::release() {
    release_node(this->from);
    release_node(this->where);

    for (var &s: *this->set) {
        release_node(s);
    }
    delete this->set;
    this->set = null;
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
        case T_INSERT:
            stmt = parse_insert();
            break;
        case T_UPDATE:
            stmt = parse_update();
            break;
        case T_WITH:
        case T_SELECT:
            stmt = parse_read();
            break;
        case T_DROP:
            stmt = parse_drop();
            break;
        case T_DELETE:
            stmt = parse_delete();
            break;
        case T_DESC:
        case T_DESCRIBE:
            stmt = parse_describe();
            break;
        case T_SHOW:
            stmt = parse_show();
            break;
        case T_BANG:
            stmt = parse_bang();
            break;
        case T_HISTORY:
            stmt = parse_history();
            break;
        default:
            stmt = Statement{.type = S_NONE};
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
