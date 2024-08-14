//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

typedef enum {
    T_EOF,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD,
    T_EQ, T_NE1, T_NE2, T_LT, T_GT, T_LE, T_GE,
    T_LPAREN, T_RPAREN, T_COMMA, T_SEMICOLON,
    T_NUMBER, T_STRING, T_IDENTIFIER,
    T_SELECT, T_AS, T_FROM, T_WHERE, T_JOIN, T_ON,
    T_GROUP, T_BY, T_ORDER, T_HAVING, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_IS, T_NULL,
    T_WHEN, T_THEN, T_ELSE, T_WITH,
} TokenType;

typedef enum {
    A_NONE,
    A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
    A_IDENTIFIER, A_FUNC_CALL,
    A_NUMBER, A_STRING,
} ASTType;

typedef struct Token Token;
typedef struct ASTNode ASTNode;

struct Token {
    TokenType type;
    double number; // for T_NUMBER
    // for T_STRING or T_IDENTIFIER, its name; for other, its description
    // so that text will never be NULL
    String *text;
};

struct ASTNode {
    ASTType type;
    union {
        struct {

        } select;
        ASTNode *from;
        // ...
    };
};

#endif //BASH_SQL_DEFS_H
