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
    T_GROUP, T_BY, T_ORDER, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_IS, T_NULL,
    T_WHEN, T_THEN, T_ELSE, T_WITH,
} TokenType;

typedef enum {
    P_STRING, P_INTEGER, P_REAL,
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
    union {
        long integer; // for T_INTEGER
        double real; // for T_REAL
    };
    // for T_STRING or T_IDENTIFIER, its name; for other, its description
    // so that text will never be NULL
    String text;
};

struct Param {
    ParamType type;
    union {
        String str;
        long integer;
        double real;
    };

    Param(ParamType type, String str) : type(type), str(std::move(str)) {};

    Param(ParamType type, long value) : type(type), integer(value) {};

    Param(ParamType type, double value) : type(type), real(value) {};
};

struct Column {
    ASTNode *table;
    int index;
};

struct ASTNode {
    ASTType type;
    /*
     * []: is a list
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
    Vector<ASTNode *> *list; // subquery list/col list/order list
    union {
        ASTNode *select{};
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
            Column col;
            bool asc;
        } order;
        struct {
            int count;
            int offset;
        } limit;
    }; // data

    explicit ASTNode(ASTType type = A_NONE) : type(type), left(null), mid(null), right(null), list(null) {}
};

struct WithNode {
    SelectStatement *stmt;
    String as;
};

struct SelectNode {
    Column *col;
    String as;
};

struct FromNode {
};

struct WhereNode {
};

struct GroupNode {
};

struct OrderNode {
    String col;
    bool asc = true;
};

struct LimitNode {
    int offset;
    int limit;
};

struct SelectStatement {
    Vector<WithNode> *with;
    Vector<SelectNode> *select;
    FromNode *from;
    WhereNode *where;
    Vector<String> *group;
    Vector<OrderNode> *order;
    LimitNode *limit;
};

#endif //BASH_SQL_DEFS_H
