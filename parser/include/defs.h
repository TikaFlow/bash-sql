//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

typedef enum {
    T_EOF,
    T_PLUS, T_MINUS, T_STAR, T_SLASH,
    T_MOD, T_UNDERSCORE,
    T_LPAREN, T_RPAREN,
    T_NUMBER, T_STRING, T_IDENTIFIER,
    T_SELECT, T_FROM, T_WHERE, T_JOIN, T_ON, T_AS, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT,
    T_COMMA,
    T_SEMICOLON,
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
    char *text;
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
