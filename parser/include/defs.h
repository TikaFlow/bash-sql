//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

// table name, column name
using TableColumn = std::pair<String, String>;
// column type, column index in the table(1-indexed)
using ColumnCount = std::pair<int, int>;
// type of result set
using Table = std::pair<Map<TableColumn, ColumnCount> *, Map<String, int> *>;

/**
 * select statement
 * @note writing order: with -> select -> from -> join..on -> where -> group -> order -> limit
 * @note execution order: with -> from -> join..on -> where -> group -> select -> order -> limit
 */
typedef struct SelectStatement SelectStatement;
typedef struct WithNode WithNode;
typedef struct SelectNode SelectNode;
typedef struct FromNode FromNode;
typedef struct WhereNode WhereNode;
typedef struct GroupNode GroupNode;
typedef struct OrderNode OrderNode;
typedef struct LimitNode LimitNode;
typedef struct Token Token;
typedef struct Param Param;
typedef struct Column Column;
typedef struct ASTNode ASTNode;

typedef enum {
    T_EOF,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD,
    T_EQ, T_NE1, T_NE2, T_LT, T_GT, T_LE, T_GE,
    T_LPAREN, T_RPAREN, T_COMMA, T_SEMICOLON,
    T_INTEGER, T_REAL, T_STRING, T_IDENTIFIER,
    T_SELECT, T_AS, T_FROM, T_WHERE, T_JOIN, T_ON,
    T_GROUP, T_BY, T_ORDER, T_ASC, T_DESC, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_IS, T_NULL, T_WITH,
} TokenType;

typedef enum {
    D_STRING, D_INT, D_REAL, D_BOOL,
} DataType;

typedef enum {
    A_FUNC_CALL,
    A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    A_AND, A_OR, A_NOT, A_ISNULL, A_NOTNULL, A_IN, A_NOTIN,
} ASTType;

struct Token {
    TokenType type;
    union {
        long integer; // for T_INTEGER
        double real; // for T_REAL
    };
    // for T_STRING or T_IDENTIFIER, its name; for other, its description
    // so that text will never be NULL
    String text;
};

struct Param {
    DataType type;
    union {
        String s;
        long l;
        double d;
        bool b;
    };

    explicit Param(String str) : type(D_STRING), s(std::move(str)) {};

    explicit Param(long value) : type(D_INT), l(value) {};

    explicit Param(double value) : type(D_REAL), d(value) {};

    explicit Param(bool value) : type(D_BOOL), b(value) {};
};

struct Column {
    ASTNode *table;
    int index;
};

struct ASTNode {
    ASTType atype;
    DataType dtype;
    ASTNode *left;
    ASTNode *mid;
    ASTNode *right;
};

struct WithNode {
    SelectStatement *stmt;
    String as;
};

struct SelectNode {
    ASTNode *col; // Column expression
    String as;
};

struct FromNode {
};

struct WhereNode {
};

struct GroupNode {
    // Column expression, directly included in SelectStatement
};

struct OrderNode {
    int index;
    bool asc;
};

struct LimitNode {
    int offset;
    int count;
};

struct SelectStatement {
    Vector<WithNode> *with;
    Vector<SelectNode> *select;
    FromNode *from;
    WhereNode *where;
    Vector<ASTNode *> *group;
    Vector<OrderNode> *order;
    LimitNode *limit;
};

#endif //BASH_SQL_DEFS_H
