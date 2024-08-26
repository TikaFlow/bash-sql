//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

/**
 * select statement
 * @note writing order: with -> select -> from -> where -> group -> order -> limit
 * @note execution order: with -> from -> where -> group -> select -> order -> limit
 */
typedef struct SelectStatement SelectStatement;
typedef struct WithNode WithNode;
typedef struct SelectNode SelectNode;
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
    T_SELECT, T_AS, T_FROM, T_WHERE,
    T_GROUP, T_BY, T_ORDER, T_ASC, T_DESC, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_IS, T_NULL, T_WITH,
} TokenType;

typedef enum {
    D_NONE, // represents to be determined
    D_STRING, D_INT, D_REAL, D_BOOL,
    D_NUMBER, // in function, check if is real or int
} DataType;

typedef enum {
    A_FUNC_CALL, A_PARAM, A_LITERAL, A_COLUMN,
    A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
    A_NEGATE, A_NOT, A_ISNULL, A_NOTNULL, // unary operator
} ASTType;

// column type, column index in the table(0-indexed), -1 if ambiguous
using ColumnDesc = Pair<DataType, int>;
// column name, columns type
using Table = Vector<Pair<String, DataType>>;

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
        long l;
        double d;
        bool b;
    };
    String s;

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
    ASTNode *right;
    union {
        long l;
        double d;
        bool b;
    }; // value
    String s;

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right) :
            atype(atype), dtype(dtype), left(left), right(right), l(0) {}

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right, long l) :
            atype(atype), dtype(dtype), left(left), right(right), l(l) {}

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right, double d) :
            atype(atype), dtype(dtype), left(left), right(right), d(d) {}

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right, String s) :
            atype(atype), dtype(dtype), left(left), right(right), s(std::move(s)), l(0) {}
};

struct WithNode {
    SelectStatement *stmt;
    String as;
};

struct SelectNode {
    ASTNode *col; // Column expression
    String as;
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
    Map<String, ColumnDesc> *from;
    ASTNode *where;
    Vector<int> *group;
    Vector<OrderNode> *order;
    LimitNode *limit;
};

#endif //BASH_SQL_DEFS_H
