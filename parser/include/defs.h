//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

typedef struct Token Token;
typedef struct Param Param;
typedef struct Column Column;
typedef struct ASTNode ASTNode;

typedef enum {
    T_EOF,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD,
    T_EQ, T_NE1, T_NE2, T_LT, T_GT, T_LE, T_GE,
    T_LPAREN, T_RPAREN, T_COMMA, T_SEMICOLON,
    T_NUMBER, T_STRING, T_IDENTIFIER,
    T_SELECT, T_AS, T_FROM, T_WHERE, T_JOIN, T_ON,
    T_GROUP, T_BY, T_ORDER, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_IS, T_NULL,
    T_WHEN, T_THEN, T_ELSE, T_WITH,
} TokenType;

typedef enum {
    P_STRING, P_NUMBER,
} ParamType;

typedef enum {
    A_NONE,
    A_SELECT_STMT, A_WITH, A_SELECT, A_FROM, A_JOIN, A_WHERE, A_GROUP,
    A_ORDER, A_LIMIT, A_COL, A_FUNC, A_EXPRESSION,
    A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
    A_CONDITION, A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    A_AND, A_OR, A_NOT, A_ISNULL, A_NOTNULL, A_IN, A_NOTIN,
    A_WHEN, A_THEN, A_ELSE,
} ASTType;

struct Token {
    TokenType type;
    double number; // for T_NUMBER
    // for T_STRING or T_IDENTIFIER, its name; for other, its description
    // so that text will never be NULL
    String text;
};

struct Param {
    ParamType type;
    union {
        String str;
        double num;
    };
};

struct Column {
    ASTNode *table;
    String name;
};

struct ASTNode {
    ASTType type;
    /*
     * []: use next pointer
     * for select: left = with..., mid = from..., right = select... // SELECT_STMT
     *
     * for with...: data(select...[]) // WITH
     * for from...: left = ...join..., right = where... // FROM
     * for select...: left = cols..., mid = order by..., right = limit... // SELECT
     *
     * for join...: tree(and) // JOIN
     * for where...: left = tree(and/or), right = group by... // WHERE
     * for cols...: data(col[]) // COL
     * for order by...: data(order[]) // ORDER
     * for limit...: data(offset, limit) // LIMIT
     *
     * for tree: and/or(left, right) // LOGIC_AND/LOGIC_OR
     * for group by...: data(col[]) // GROUP
     * for col: data(func(col), as) // COL
     * for order: data(col, asc/desc) // ORDER
     *
     * for func: data(name, args<>) // FUNC
     */
    ASTNode *left;
    ASTNode *mid;
    ASTNode *right;
    ASTNode *next; // next in list
    union {
        ASTNode *select;
        union {
            // data  may be one of: literal, func_call, raw col
            String str;
            double num;
            ASTNode *call;
            Column col;
        } condition;
        union {
            struct {
                String name;
                Vector<Param> *args;
            } call;
            Param literal;
        } func; // also used in literal expression
        struct {
            union {
                ASTNode *call; // if a func_call
                Column col; // or raw col
            };
            String as;
        } col; // also used in group by...
        struct {
            String name;
            bool asc;
        } order;
        struct {
            int count;
            int offset;
        } limit;
    }; // data

    ASTNode(ASTType type = A_NONE) : type(type) {}
};

#endif //BASH_SQL_DEFS_H
