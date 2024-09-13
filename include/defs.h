//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

#include <cmath>

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
    T_SELECT, T_AS, T_FROM, T_JOIN, T_ON, T_WHERE,
    T_GROUP, T_BY, T_ORDER, T_ASC, T_DESC, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_LIKE, T_IS, T_NULL, T_WITH,
} TokenType;

typedef enum {
    D_NONE, // no type, or to be determined
    D_STRING, D_BOOL, D_NUMBER,
} DataType;

typedef enum {
    A_JOIN,
    A_FUNC_CALL, A_PARAM, A_LITERAL, A_COLUMN,
    A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
    A_NEGATE, A_NOT, A_ISNULL, A_LIKE, // unary operator
} ASTType;

// column type, column index in the table(0-indexed), -1 if ambiguous
using ColumnDesc = Pair<DataType, int>;
// column name, columns type
using Table = Vector<Pair<String, DataType>>;
using TableSet = Map<String, ColumnDesc>;

struct Token {
    TokenType type;
    union {
        int integer; // for T_INTEGER
        double real; // for T_REAL
    };
    // for T_STRING or T_IDENTIFIER, its name; for other, its description
    // so that text will never be NULL
    String text;
};

struct ASTNode {
    ASTType atype;
    DataType dtype;
    ASTNode *left;
    ASTNode *right;
    double number = 0;
    String text; // when d_number, "<null>"; when d_string, its value

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right) :
            atype(atype), dtype(dtype), left(left), right(right) {}

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right, double num) :
            atype(atype), dtype(dtype), left(left), right(right), number(num) {}

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right, String str) :
            atype(atype), dtype(dtype), left(left), right(right), text(std::move(str)) {}

    bool tableless() const {
        if (atype == A_COLUMN) {
            return false;
        }

        if (left) {
            if (!left->tableless()) {
                return false;
            }
        }

        if (right) {
            if (!right->tableless()) {
                return false;
            }
        }

        return true;
    }
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
    /**
     *      join
     *     /   \
     *   join   on(condition)
     *   /   \
     * ...   on(condition)
     */
    ASTNode *from;
    ASTNode *where;
    Vector<ASTNode *> *group;
    Vector<OrderNode> *order;
    LimitNode *limit;

    bool tableless_with() const {
        if (!with) {
            return true;
        }

        return all_of(with->begin(), with->end(), [](WithNode &node) { return node.stmt->tableless(); });
    }

    bool tableless_select() const {
        if (!select) {
            return true;
        }

        return all_of(select->begin(), select->end(), [](SelectNode &node) { return node.col->tableless(); });
    }

    bool tableless() const {
        return tableless_with() && tableless_select();
    }
};

#endif //BASH_SQL_DEFS_H
