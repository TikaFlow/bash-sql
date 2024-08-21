//
// Created by tika on 24-8-14.
//

#include "parse.h"

// next token index to be process
static int INDEX = 0;
// token vector from lexer
static const Vector<Token *> *TOKEN;
// function name, params vector
static Map<String, Vector<Param *>> FUNC;
// table.column, columns desc
static Map<TableColumn, ColumnCount> TABLE;
// column count
static Map<String, int> COLUMN;
// init column number, 32 by default
extern int COL_NUM;

/**
 * initialize the table map and function map
 */
static void init_map() {
    // table std by default
    for (int i = 1; i <= COL_NUM; ++i) {
        String col = "col" + to_string(i);
        TABLE[{"std", col}] = {D_STRING, i};
        COLUMN[col] = 1;
    }

    using ParamList = Vector<Param *>;
    // TODO add functions
}

/**
 * initialize the table map for a result set
 * @param use_std use std table or not
 * @return table map
 */
static Table init_table(bool use_std = false) {
    var table_col = new Map<TableColumn, ColumnCount>();
    var col_count = new Map<String, int>();

    if (use_std) {
        table_col->insert(TABLE.begin(), TABLE.end());
        col_count->insert(COLUMN.begin(), COLUMN.end());
    }

    return {table_col, col_count};
}

/**
 * peek one token
 * @return token, null if no more token
 */
static Token *peek() {
    static val size = TOKEN->size();
    if (INDEX >= size) {
        return null;
    }
    return TOKEN->at(INDEX);
}

/**
 * pop one token
 * @return token, null if no more token
 */
static Token *pop() {
    Token *token = peek();
    if (token) {
        INDEX++;
    }
    return token;
}

/**
 * push back one token
 */
static void unpop() {
    if (INDEX > 0) {
        INDEX--;
    }
}

/**
 * check if the next token is the expected type
 * @param type expected type
 * @param what what to show if the type is not matched
 */
static void match(TokenType type, const String &what) {
    Token *token = pop();
    if (token->type == type) {
        return;
    } else {
        show_error("Expected " + what + " but got '" + token->text + "'");
    }
}

/**
 * parse SELECT clause
 * @return vector of SELECT node
 * @todo implement
 */
static Vector<SelectNode> *parse_select() {
    return new Vector<SelectNode>();
}

/**
 * parse FROM clause
 * @return pointer to FROM node
 * @todo implement
 */
static FromNode *parse_from() {
    return new FromNode();
}

/**
 * generate from node of "from std", when no from clause, use std
 * @return pointer to FROM node
 * @todo implement
 */
static FromNode *from_std() {
    return new FromNode();
}

/**
 * get column index from SELECT clause, which starts from 1
 * @param stmt SELECT clause
 * @return index of the column in SELECT clause
 * @todo implement
 */
static int parse_order_col(const String &col, Vector<SelectNode> *stmt) {
    return 1;
}

/**
 * parse ORDER BY clause
 * @param stmt SELECT clause
 * @return vector of ORDER BY node
 */
static Vector<OrderNode> *parse_order_by(Vector<SelectNode> *stmt) {
    match(T_BY, "by");

    val node = new Vector<OrderNode>();
    int order;
    bool asc;
    do {
        var token = pop();
        if (token->type == T_INTEGER) {
            order = (int) token->integer;
        } else if (token->type == T_STRING) {
            order = parse_order_col(token->text, stmt);
        } else {
            show_error("Expected column name or index but got " + token->text);
        }

        asc = true;
        token = pop();
        if (token->type == T_DESC) {
            asc = false;
        } else if (token->type != T_ASC) {
            unpop();
        }

        node->push_back({order, asc});
    } while (pop()->type == T_COMMA);
    unpop();

    return node;
}

/**
 * parse LIMIT clause, support format:
 * LIMIT count
 * LIMIT offset, count
 * LIMIT count OFFSET offset
 * @return limit node
 */
static LimitNode *parse_limit() {
    val node = new LimitNode();
    match(T_INTEGER, "integer");
    long count = pop()->integer, offset = 0;
    TokenType type = peek()->type;
    if (type == T_COMMA || type == T_OFFSET) {
        pop(); // comma/offset
        match(T_INTEGER, "integer");
        offset = pop()->integer;
        if (type == T_COMMA) {
            long temp = offset;
            offset = count;
            count = temp;
        }
    }

    node->count = (int) count;
    node->offset = (int) offset;

    return node;
}

/**
 * parse single SELECT statement
 * @param with WITH clause
 * @return pointer to SELECT statement
 */
static SelectStatement *parse_select_stmt(Vector<WithNode> *with = null) {
    val stmt = new SelectStatement();

    stmt->with = with;
    stmt->select = parse_select();

    if (peek()->type == T_FROM) {
        pop();
        stmt->from = parse_from();
    } else {
        stmt->from = from_std();
    }

    if (peek()->type == T_WHERE) {
        pop();
        // TODO where clause
        // stmt->where =
    }

    if (peek()->type == T_GROUP) {
        pop();
        // TODO group by clause
        // stmt->group =
    }

    if (peek()->type == T_ORDER) {
        pop();
        stmt->order = parse_order_by(stmt->select);
    }
    if (peek()->type == T_LIMIT) {
        pop();
        stmt->limit = parse_limit();
    }

    return stmt;
}

/**
 * parse WITH clause
 * @return vector of WITH nodes
 */
static Vector<WithNode> *parse_with_queries() {
    val with = new Vector<WithNode>();
    SelectStatement *query;

    do {
        val temp = peek()->text;
        match(T_IDENTIFIER, "temporary table name");
        match(T_AS, "as");
        match(T_LPAREN, "open parentheses");
        match(T_SELECT, "select clause");
        query = parse_select_stmt();
        match(T_RPAREN, "close parentheses");

        with->push_back({query, temp});
    } while (pop()->type == T_COMMA);
    unpop();

    return with;
}

/**
 * parse SELECT statement starting with WITH
 * @return pointer to SELECT statement
 */
static SelectStatement *parse_with() {
    val with = parse_with_queries();

    match(T_SELECT, "select clause");
    return parse_select_stmt(with);
}

/**
 * parse SELECT statements separated by semicolon
 * @return vector of SELECT statements
 */
static Vector<SelectStatement *> *parse() {
    SelectStatement *stmt;
    Token *start;
    val queries = new Vector<SelectStatement *>();
    while ((start = pop())) {
        if (start->type == T_SELECT) {
            stmt = parse_select_stmt();
        } else if (start->type == T_WITH) {
            stmt = parse_with();
        } else {
            show_error("Expected 'select' or 'with' but got " + start->text);
        }
        match(T_SEMICOLON, "semicolon at the end of sql statement");

        queries->push_back(stmt);
    }

    return queries;
}

/**
 * parse SQL statements from tokens
 * @param tokens tokens
 * @return vector of SQL statements
 */
Vector<SelectStatement *> *parse(Vector<Token *> *tokens) {
    TOKEN = tokens;
    init_map();
    return parse();
}
