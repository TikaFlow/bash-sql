//
// Created by tika on 24-8-14.
//

#include "parse.h"

static int INDEX = 0;
static const Vector<Token *> *TOKEN;
static Map<String, int> FUNC;

/**
 * initialize the function map
 */
static void init_map() {
    FUNC["sum"] = 1;
    FUNC["avg"] = 1;
    FUNC["count"] = 1;
    FUNC["max"] = 1;
    FUNC["min"] = 1;
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
            tail->next = order;
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
 * parse SELECT statement behind WITH clause(maybe there is none)
 * or sub SELECT statement in WITH clause
 * @param is_with true if it is in WITH clause
 * @param stmt node of SELECT statement
 * @return node of SELECT statement
 */
static ASTNode *parse_select_stmt(bool is_with, ASTNode *stmt = null) {
    if (!stmt) {
        stmt = new ASTNode(is_with ? A_WITH : A_SELECT_STMT);
    }

    stmt->right = new ASTNode(A_SELECT);
    val select = stmt->right;
    select->left = parse_select();

    if (peek()->type == T_FROM) {
        pop();
        stmt->mid = parse_from(select);
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
 * parse SELECT clause starting with WITH
 * @return node of SELECT statement
 */
static ASTNode *parse_with() {
    val stmt = new ASTNode(A_SELECT_STMT);
    ASTNode *head = null, *tail;
    ASTNode *with;

    do {
        match(T_IDENTIFIER, "temporary table name");
        val temp = pop()->text;
        match(T_AS, "as");
        match(T_LPAREN, "open parentheses");
        match(T_SELECT, "select clause");
        with = parse_select_stmt(true);
        match(T_RPAREN, "close parentheses");

        if (head) {
            tail->next = with;
            tail = with;
        } else {
            head = tail = with;
        }
    } while (pop()->type != T_COMMA);
    unpop();
    stmt->left = head;

    match(T_SELECT, "select clause");
    parse_select_stmt(false, stmt);

    return stmt;
}

/**
 * parse several SELECT statements separated by semicolon
 * @note writing order: with -> select -> from -> join..on -> where -> group -> order -> limit
 * @note execution order: with -> from -> join..on -> where -> group -> select -> order -> limit
 * @return head node of linked list, every node is a statement
 */
static ASTNode *parse() {
    Token *start;
    ASTNode *head = null, *tail, *node;
    while ((start = pop())) {
        if (start->type == T_SELECT) {
            node = parse_select_stmt(false);
        } else if (start->type == T_WITH) {
            node = parse_with();
        } else {
            show_error("Expected select or with but got " + start->text);
        }
        match(T_SEMICOLON, "semicolon at the end of sql statement");

        if (head) {
            tail->next = node;
            tail = node;
        } else {
            head = tail = node;
        }
    }

    return head;
}

/**
 * parse SQL statements from tokens
 * @param tokens tokens
 * @return head node of linked list, every node is a statement
 */
ASTNode *parse(Vector<Token *> *tokens) {
    TOKEN = tokens;
    init_map();
    return parse();
}
