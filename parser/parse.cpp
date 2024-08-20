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
        TABLE[{"std", col}] = {P_STRING, i};
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
 * @return true if the type is matched
 */
static bool match(TokenType type, const String &what) {
    Token *token = peek();
    if (token->type == type) {
        pop();
        return true;
    } else {
        show_error("Expected " + what + " but got '" + token->text + "'");
    }
}

/**
 * parse one column from SELECT clause
 * @param select node of SELECT clause
 * @param index index of the column
 * @return target Column
 * @todo implement
 */
static Column get_col(ASTNode *select, int index) {
    return Column{};
}

/**
 * parse one column from SELECT clause and FROM clause
 * @param name
 * @param from node of FROM clause
 * @param select node of SELECT clause,
 * if null, call is from SELECT clause; otherwise, call is from ORDER BY clause
 * @return target Column
 * @todo implement
 */
static Column get_col(const String &name, ASTNode *from, ASTNode *select = null) {
    return Column{};
}

/**
 * parse SELECT clause
 * @return node of SELECT clause
 * @todo implement
 */
static ASTNode *parse_select() {
    return new ASTNode();
}

/**
 * parse FROM clause, and rebuild column in SELECT clause
 * @param select node of SELECT clause
 * @return node of FROM clause
 * @todo implement
 */
static ASTNode *parse_from(ASTNode *select) {
    return new ASTNode();
}

/**
 * parse FROM clause, and rebuild column in SELECT clause
 * @param select node of SELECT clause
 * @return node of FROM clause
 * @todo implement
 */
static ASTNode *from_std(ASTNode *select) {
    return new ASTNode();
}

/**
 * parse one ORDER BY column with asc or desc
 * @param stmt node of SELECT statement
 * @return node of ORDER BY column
 */
static ASTNode *parse_order_col(ASTNode *stmt) {
    val node = new ASTNode(A_ORDER);
    val order = &node->order;

    val name = pop();
    if (name->type == T_INTEGER) {
        if (name->integer <= 0) {
            show_error("Expected positive integer but got " + to_string(name->integer));
        }
        order->col = get_col(stmt->right, (int) name->integer);
    } else {
        order->col = get_col(name->text, stmt->left, stmt->right);
    }

    var ascending = true;
    if (peek()->type == T_STRING) {
        val text = to_lower(pop()->text);
        if (text == "desc") {
            ascending = false;
        } else if (text != "asc") {
            show_error("Expected asc or desc but got " + text);
        }
    }
    order->asc = ascending;

    return node;
}

/**
 * parse ORDER BY clause
 * @param stmt node of SELECT statement
 * @return node of ORDER BY clause
 */
static ASTNode *parse_order_by(ASTNode *stmt) {
    match(T_BY, "by");
    ASTNode *order, *head = null, *tail;
    do {
        val col = peek();
        if (col->type == T_INTEGER || col->type == T_STRING) {
            order = parse_order_col(stmt);
        } else {
            show_error("Expected column name or index but got " + col->text);
        }

        if (head) {
            tail = order;
        } else {
            head = order;
            tail = order;
        }
    } while (pop()->type == T_COMMA);
    unpop();

    return head;
}

/**
 * parse LIMIT clause, support format:
 * LIMIT count
 * LIMIT offset, count
 * LIMIT count OFFSET offset
 * @return limit node
 */
static ASTNode *parse_limit() {
    val limit = new ASTNode(A_LIMIT);
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

    limit->limit.count = (int) count;
    limit->limit.offset = (int) offset;

    return limit;
}

/**
 * parse SELECT statement
 * @param with node of WITH clause
 * @return node of SELECT statement
 */
static ASTNode *parse_select_stmt(ASTNode *with = null) {
    val stmt = new ASTNode(A_SELECT_STMT);
    stmt->left = with;

    val select = stmt->right = new ASTNode(A_SELECT);
    select->left = parse_select();

    if (peek()->type == T_FROM) {
        pop();
        stmt->mid = parse_from(select);
    } else {
        stmt->mid = from_std(select);
    }

    if (peek()->type == T_ORDER) {
        pop();
        select->mid = parse_order_by(stmt);
    }
    if (peek()->type == T_LIMIT) {
        pop();
        select->right = parse_limit();
    }

    return stmt;
}

/**
 * parse WITH clause
 * @return node of WITH clause
 */
static ASTNode *parse_with_queries() {
    val with = new ASTNode(A_WITH);
    val list = new Vector<ASTNode *>();

    ASTNode *query;
    do {
        match(T_IDENTIFIER, "temporary table name");
        val temp = pop()->text;
        match(T_AS, "as");
        match(T_LPAREN, "open parentheses");
        match(T_SELECT, "select clause");
        query = parse_select_stmt();
        match(T_RPAREN, "close parentheses");

        list->push_back(query);
    } while (pop()->type != T_COMMA);
    unpop();

    with->list = list;

    return with;
}

/**
 * parse SELECT clause starting with WITH
 * @return node of SELECT statement
 */
static ASTNode *parse_with() {
    val left = parse_with_queries();

    match(T_SELECT, "select clause");
    return parse_select_stmt(left);
}

/**
 * parse several SELECT statements separated by semicolon
 * @return vector of SELECT statements
 */
static Vector<ASTNode *> *parse() {
    ASTNode *node;
    Token *start;
    val queries = new Vector<ASTNode *>();
    while ((start = pop())) {
        if (start->type == T_SELECT) {
            node = parse_select_stmt();
        } else if (start->type == T_WITH) {
            node = parse_with();
        } else {
            show_error("Expected select or with but got " + start->text);
        }
        match(T_SEMICOLON, "semicolon at the end of sql statement");

        queries->push_back(node);
    }

    return queries;
}

/**
 * parse SQL statements from tokens
 * @param tokens tokens
 * @return head node of linked list, every node is a statement
 */
Vector<ASTNode *> *parse(Vector<Token *> *tokens) {
    TOKEN = tokens;
    init_map();
    return parse();
}
