//
// Created by tika on 24-8-14.
//

#include "parse.h"

static int INDEX = 0;
static const Vector<Token *> *TOKEN;
static Map<String, int> FUNC;

static void init_map() {
    FUNC["sum"] = 1;
    FUNC["avg"] = 1;
    FUNC["count"] = 1;
    FUNC["max"] = 1;
    FUNC["min"] = 1;
}

static Token *peek() {
    static val size = TOKEN->size();
    if (INDEX >= size) {
        return null;
    }
    return TOKEN->at(INDEX);
}

static Token *pop() {
    Token *token = peek();
    if (token) {
        INDEX++;
    }
    return token;
}

static void unpop() {
    if (INDEX > 0) {
        INDEX--;
    }
}

static bool match(TokenType type, const String &what) {
    Token *token = peek();
    if (token->type == type) {
        pop();
        return true;
    } else {
        show_error("Expected " + what + " but got '" + token->text + "'");
    }
}

static ASTNode *parse_select() {
    return new ASTNode();
}

static ASTNode *parse_with() {
    val node = new ASTNode();

    do {
        match(T_IDENTIFIER, "temporary table name");
        val temp = pop()->text;
        match(T_AS, "as");
        match(T_LPAREN, "open parentheses");
        parse_select();
        match(T_RPAREN, "close parentheses");
    } while (pop()->type != T_COMMA);
    unpop();

    match(T_SELECT, "select clause");
    parse_select();
    match(T_SEMICOLON, "semicolon at the end of sql statement");

    return node;
}

static ASTNode *parse() {
    Token *start;
    ASTNode *head = null, *tail = null, *node = null;
    while ((start = pop())) {
        if (start->type == T_SELECT) {
            node = parse_select();
        } else if (start->type == T_WITH) {
            node = parse_with();
        } else {
            show_error("Expected select or with but got " + start->text);
        }

        if (head) {
            tail->next = node;
            tail = node;
        } else {
            head = tail = node;
        }
    }

    return head;
}

ASTNode *parse(Vector<Token *> *tokens) {
    TOKEN = tokens;
    init_map();
    return parse();
}
