//
// Created by tika on 24-8-14.
//

#include "parse.h"

static int INDEX = 0;
static const Vector<Token *> *TOKEN;
static Map<String, int> FUNC;

static void init_map(){
    FUNC["sum"] = 1;
    FUNC["avg"] = 1;
    FUNC["count"] = 1;
    FUNC["max"] = 1;
    FUNC["min"] = 1;
}

static Token *next() {
    return TOKEN->at(INDEX++);
}

static void prev() {
    INDEX--;
}

static bool match(TokenType type, String &what) {
    if (TOKEN->at(INDEX)->type == type) {
        next();
        return true;
    } else {
        show_error("Expected " + what + " but got " + TOKEN->at(INDEX)->text);
    }
}

static ASTNode *parse_select() {

}

static ASTNode *parse_with() {

}

static ASTNode *parse() {
    Token *first = next();
    if (first->type == T_SELECT) {
        return parse_select();
    } else if (first->type == T_WITH) {
        return parse_with();
    } else {
        show_error("Expected select or with but got " + first->text);
    }
}

ASTNode *parse(Vector<Token *> *tokens) {
    TOKEN = tokens;
    init_map();
    return parse();
}
