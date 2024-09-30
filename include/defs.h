//
// Created by tika on 24-8-13.
//

#ifndef BASH_SQL_DEFS_H
#define BASH_SQL_DEFS_H

#include "global.h"

typedef enum {
    T_EOF, T_AT, T_BANG, T_HISTORY,
    T_PLUS, T_MINUS, T_STAR, T_SLASH,
    T_EQ, T_NE1, T_NE2, T_LT, T_GT, T_LE, T_GE,
    T_LPAREN, T_RPAREN, T_COMMA, T_SEMICOLON,
    T_INTEGER, T_REAL, T_STRING, T_IDENTIFIER,
    T_SELECT, T_AS, T_FROM, T_JOIN, T_ON, T_WHERE, T_TRUE, T_FALSE,
    T_GROUP, T_BY, T_ORDER, T_ASC, T_DESC, T_IN, T_OFFSET, T_LIMIT,
    T_AND, T_OR, T_NOT, T_LIKE, T_IS, T_NULL, T_WITH, T_REGEXP,
    T_CREATE, T_TABLE, T_DROP, T_SHOW, T_DESCRIBE, T_TABLES,
    T_INSERT, T_UPDATE, T_DELETE, T_SET, T_VALUES, T_INTO,
} TokenType;

typedef enum {
    S_NONE,
    S_BANG, S_HISTORY,
    S_SELECT, S_INSERT, S_UPDATE, S_DELETE, // DML
    S_CREATE, S_DROP, S_SHOW, S_DESCRIBE, // DDL
} StatementType;

typedef enum {
    D_NONE, // no type, or to be determined
    D_NULL, D_STRING, D_BOOL, D_INTEGER, D_REAL,
    D_NUMBER,
} DataType;

typedef enum {
    A_JOIN,
    A_FUNC_CALL, A_PARAM, A_LITERAL, A_COLUMN,
    A_ADD, A_SUB, A_MUL, A_DIV,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
    A_NEGATE, A_NOT, A_ISNULL, A_LIKE, A_REGEXP, // unary operator
} ASTType;

struct Token {
    TokenType type;
    union {
        int integer; // for T_INTEGER
        double real; // for T_REAL
    };
    // for T_STRING or T_IDENTIFIER, its name; for other, its description
    // so that text will never be NULL
    String text;

    String to_string() const;
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

    ASTNode(ASTType atype, DataType dtype, ASTNode *left, ASTNode *right, const String &text) :
            atype(atype), dtype(dtype), left(left), right(right), text(text) {}

    bool tableless() const;

    String to_string() const;
};

struct Cell {
    DataType type;
    double number = 0;
    String text;

    Cell() : type(D_NULL) {};

    Cell(const Cell &cell) = default;

    Cell(DataType type, double num) : type(type), number(num) {};

    explicit Cell(const String &str) : type(D_STRING), text(str) {};

    String to_string() const;
};

struct SelectStatement;
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

/**
 * select statement
 * @note writing order: with -> select -> from -> where -> group -> order -> limit
 * @note execution order: with -> from -> where -> group -> select -> order -> limit
 */
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

    bool tableless_with() const;

    bool tableless_select() const;

    bool tableless() const;

    void release();
};

struct DeleteStatement {
    ASTNode *table;
    ASTNode *where;

    void release();
};

struct UpdateStatement {
    ASTNode *table;
    Vector<ASTNode *> *set;
    ASTNode *where;

    void release();
};

struct InsertStatement {
    ASTNode *table;
    // index is the column index, value is the index in values
    // -1 if not specified, null means all columns in order
    Vector<int> *columns;
    bool use_query;
    union {
        // data may from literal or query
        Vector<Vector<ASTNode *> *> *values;
        SelectStatement *query;
    };

    void release();
};

struct Statement {
    StatementType type;
    union {
        SelectStatement *stmt_select;
        DeleteStatement *stmt_delete;
        UpdateStatement *stmt_update;
        InsertStatement *stmt_insert;
        int number;
    };
    String name;

    void release();
};

struct ProgramOptions {
    bool title;
    bool line_no;
    bool interactive;
    String data;
    char delimiter;
    int columns;
    String sql;
};

using Row = Vector<Cell *>;
using Result = Vector<Row *>;
// column type, column index in the table(0-indexed), -1 if ambiguous
using ColumnDesc = Pair<DataType, int>;
// column name, columns type
using Schema = Vector<Pair<String, DataType>>;
using TableSet = Map<String, ColumnDesc>;

extern ProgramOptions *options;
extern Map<String, Pair<Schema *, Result *>> *db;
extern Vector<String> history;

#endif //BASH_SQL_DEFS_H
