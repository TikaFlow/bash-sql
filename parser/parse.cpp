//
// Created by tika on 24-8-14.
//

#include "parse.h"

// next token index to be process
static int INDEX = 0;
// token vector from lexer
static const Vector<Token *> *TOKENS;
// build-in functions
static const Map<String, DataType> FUNCS = {
        {"sub_str", D_STRING},
        {"lower",   D_STRING},
        {"upper",   D_STRING},
        {"trim",    D_STRING},
};
// maintain 2 lists for every select
// columns of "from" set
// aliases, index
static Map<String, int> ALIAS;
// columns selected
static Table *SELECTS;
// all tables
static Map<String, Table *> TABLES;
// init column number, 32 by default
extern int COL_NUM;

static ASTNode *expression();

/**
 * peek one token
 * @return token, null if no more token
 */
static Token *peek() {
    static val size = TOKENS->size();
    if (INDEX >= size) {
        return null;
    }
    return TOKENS->at(INDEX);
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
 * @return the expected token, return value would never be null
 */
static Token *match(TokenType type, const String &what) {
    Token *token = pop();
    if (token && token->type == type) {
        return token;
    } else {
        show_error("Expected " + what + " but got '" + (token ? token->text : "nothing") + "'");
    }
    return null; // make compiler happy
}

/**
 * initialize the std table
 */
static void init_std() {
    val std = new Table();

    if (COL_NUM < 1) {
        COL_NUM = 32;
    }
    for (int i = 0; i < COL_NUM; ++i) {
        std->push_back({"col" + std::to_string(i + 1), D_STRING});
    }

    TABLES.insert({"std", std});
}

/**
 * prepare the table for parsing
 */
static void stmt_start() {
    ALIAS.clear();
    SELECTS = new Table();
}

/**
 * compatible data type of two sides
 * @param left left data type
 * @param right right data type
 * @return compatible data type
 */
static DataType cast_type(ASTNode *left, ASTNode *right, ASTType op_type) {
    DataType left_type = left->dtype, right_type = right->dtype;

    if (op_type >= A_ADD && op_type <= A_MOD) { // mathematical operator
        if (left_type == D_STRING || right_type == D_STRING || left_type == D_BOOL || right_type == D_BOOL) {
            show_error("incompatible operands");
        }

        if (left_type == D_REAL || right_type == D_REAL) {
            return D_REAL;
        }

        return D_INT;
    } else if (op_type == A_EQ || op_type == A_OR) { // logic operator
        if (left_type == D_BOOL && right_type == D_BOOL) {
            return D_BOOL;
        }
        show_error("incompatible operands");
    }

    return D_STRING; // make compiler happy
}

/**
 * resolve string to table str and column str
 * @param str string to resolve
 * @return pair of table name and column name
 */
static std::pair<String, String> *resolve_column(const String &str) {
    val pos = str.find('.');

    if (pos == npos) {
        return new std::pair<String, String>{"", str};
    }
    return new std::pair<String, String>{str.substr(0, pos), str.substr(pos + 1)};
}

/**
 * parse function call
 * @note function_call ::= identifier "(" [expression {"," expression}] ")"
 * @param name function name
 * @return ast node of the function call
 */
static ASTNode *function_call(const String &name) {
    ASTNode *node, *left = null, *param;
    String func = to_lower(name);
    if (!FUNCS.count(func)) {
        show_error("Unknown function '" + func + "'");
    }

    pop(); // pop "("
    if (peek()->type != T_RPAREN) {
        param = expression();
        var tk = peek();
        while (tk->type == T_COMMA) {
            pop();
            left = new ASTNode(A_PARAM, D_NONE, left, param);
        }
    }
    match(T_RPAREN, "close parenthesis");
    node = new ASTNode(A_FUNC_CALL, FUNCS.at(func), left, null);

    return node;
}

/**
 * parse primary expression
 * @note primary ::= number | string | identifier | function_call | "(" expression ")"
 * @return ast node of the primary expression
 */
static ASTNode *primary() {
    String name;
    ASTNode *node;

    var tk = peek();
    switch (tk->type) {
        case T_INTEGER:
            node = new ASTNode(A_LITERAL, D_INT, null, null, tk->integer);
            break;
        case T_REAL:
            node = new ASTNode(A_LITERAL, D_REAL, null, null, tk->real);
            break;
        case T_STRING:
            node = new ASTNode(A_LITERAL, D_STRING, null, null, tk->text);
            break;
        case T_IDENTIFIER:
            name = pop()->text;
            if (peek()->type == T_LPAREN) {
                node = function_call(name);
            } else {
                // todo check if the column exists and get the type
                node = new ASTNode(A_COLUMN, D_NONE, null, null, name);
            }
            break;
        case T_LPAREN:
            pop();
            node = expression();
            match(T_RPAREN, "close parenthesis");
            break;
        default:
            show_error("Expected primary expression but got '" + tk->text + "'");
            break;
    }

    return node;
}

/**
 * parse factor in expression
 * @note factor ::= ["+" | "-"] primary
 * @return ast node of the factor
 */
static ASTNode *factor() {
    val tk = peek();
    if (tk->type == T_PLUS || tk->type == T_MINUS) {
        pop();
    }
    var left = primary();

    if (tk->type == T_MINUS) {
        left = new ASTNode(A_NEGATE, left->dtype, left, null);
    }
    return left;
}

/**
 * parse term in expression
 * @note term ::= factor { ( "*" | "/" | "%") factor }
 * @return ast node of the term
 */
static ASTNode *term() {
    var left = factor();
    var tk = peek();

    while (tk && (tk->type == T_STAR || tk->type == T_SLASH || tk->type == T_MOD)) {
        pop();
        val right = factor();
        val atype = tk->type == T_STAR ? A_MUL :
                    (tk->type == T_SLASH ? A_DIV : A_MOD);
        val dtype = cast_type(left, right, atype);
        left = new ASTNode(atype, dtype, left, right);
        tk = peek();
    }

    return left;
}

/**
 * parse column expression in select clause
 * @note expression ::= term { ( "+" | "-" ) term }
 * @return ast node of the column expression
 */
static ASTNode *expression() {
    var left = term();
    var tk = peek();

    while (tk && (tk->type == T_PLUS || tk->type == T_MINUS)) {
        pop();
        val right = term();
        val atype = tk->type == T_PLUS ? A_ADD : A_SUB;
        val dtype = cast_type(left, right, atype);
        left = new ASTNode(atype, dtype, left, right);
        tk = peek();
    }

    return left;
}

/**
 * expand "*" to all columns in the table
 * @param select vector of select node
 * @todo implement
 */
static void expand_star(Vector<SelectNode> *select) {

}

/**
 * add alias to list and avoid conflict
 * @param alias alias name
 */
static void add_alias(const String &alias) {
    String name = to_lower(alias);
    if (ALIAS.count(name)) {
        show_error("Duplicate alias name: " + name);
    }

    ALIAS.insert({name, SELECTS->size()});
    SELECTS->emplace_back(name, D_NONE);
}

/**
 * parse SELECT clause
 * @return vector of SELECT node
 */
static Vector<SelectNode> *parse_select() {
    val select = new Vector<SelectNode>();
    do {
        if (peek()->type == T_STAR) {
            pop();
            expand_star(select);
            if (pop()->type == T_COMMA) {
                continue;
            }
            break;
        }

        val col = expression();
        String as;
        if (col->atype == A_COLUMN) {
            as = resolve_column(col->s)->second;
        }
        if (peek()->type == T_AS) {
            pop();
            as = match(T_STRING, "column alias")->text;
        }
        add_alias(as);

        select->push_back({col, as});
    } while (pop()->type == T_COMMA);
    unpop();

    return select;
}

/**
 * parse FROM clause
 * @return pointer to FROM node
 */
static Map<String, ColumnDesc> *parse_from() {
    val from = new Map<String, ColumnDesc>();
    var index = 0;
    val alias = new Map<String, int>;

    do {
        val name = match(T_IDENTIFIER, "table name")->text;
        String as;

        if (!TABLES.count(name)) {
            show_error("Table " + name + " not found");
        }
        alias->insert({name, 1}); // 1 is meaningless

        if (peek()->type == T_AS) {
            pop();
            as = match(T_IDENTIFIER, "alias name")->text;
            if (alias->count(as)) {
                show_error("Duplicate alias name: " + as);
            }
            alias->insert({as, 1});
        }

        val table = TABLES.at(name);
        for (val &col: *table) {
            from->insert({name + "." + col.first, {col.second, index}});
            from->insert({as + "." + col.first, {col.second, index}});
            if (from->count(col.first)) {
                from->insert({col.first, {col.second, -1}});
            } else {
                from->insert({col.first, {col.second, index}});
            }
            index++;
        }
    } while (pop()->type == T_COMMA);
    unpop();

    return from;
}

/**
 * generate from node of "from std", when no from clause, use std as default
 * @return pointer to FROM node
 */
static Map<String, ColumnDesc> *from_std() {
    val from = new Map<String, ColumnDesc>();
    var index = 0;

    val table = TABLES.at("std");
    for (val &col: *table) {
        // std.col1 is always valid because table name are unique and col1 is unique in table std
        from->insert({"std." + col.first, {col.second, index}});
        if (from->count(col.first)) {
            // if col1 is already in the map, insert with -1 which means it's ambiguous
            from->insert({col.first, {col.second, -1}});
        } else {
            // if col1 is unique for now, then has the same index with std.col1
            from->insert({col.first, {col.second, index}});
        }
        index++;
    }

    return from;
}

/**
 * check if the column is valid
 * and repair types if needed
 * @param from FROM table
 * @param node the node containing the column
 * @todo implement
 */
static void check_column(Map<String, ColumnDesc> *from, ASTNode *node) {
    if (!node) {
        return;
    }
}

/**
 * check if the select columns are valid
 * @param from FROM table
 * @param select SELECT clause
 */
static void validate_select(Map<String, ColumnDesc> *from, Vector<SelectNode> *select) {
    // assert(select != null); // select would never be null
    for (val &node: *select) {
        check_column(from, node.col);
    }
}

/**
 * check if the WHERE columns are valid
 * @param from FROM table
 * @param where WHERE clause
 */
static void validate_where(Map<String, ColumnDesc> *from, ASTNode *where) {
    check_column(from, where);
}

/**
 * parse GROUP BY clause
 * @return vector of GROUP BY column expression
 */
static Vector<int> *parse_group_by() {
    val node = new Vector<int>();
    match(T_BY, "by");

    do {
        val index = (int) match(T_INTEGER, "column index")->integer - 1;
        if (index < 0) {
            show_error("Column index must be greater than 0");
        }
        node->push_back(index);
    } while (pop()->type == T_COMMA);
    unpop();

    return node;
}

/**
 * get column index from SELECT clause, which starts from 0
 * @return index of the column in SELECT clause
 */
static int parse_order_col(const String &col) {
    int order = -1;
    String name = to_lower(col);

    if (ALIAS.count(name)) {
        order = ALIAS[name];
    }

    if (order == -1) {
        show_error("Column " + name + " not found in select clause");
    }
    return order;
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
            order = (int) token->integer - 1;
            if (order < 0) {
                show_error("Column index must be greater than 0");
            }
        } else if (token->type == T_STRING) {
            order = parse_order_col(token->text);
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
    long count = match(T_INTEGER, "integer")->integer, offset = 0;
    TokenType type = peek()->type;
    if (type == T_COMMA || type == T_OFFSET) {
        pop(); // comma/offset
        offset = match(T_INTEGER, "integer")->integer;
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
static SelectStatement *parse_select_stmt(const String &name, Vector<WithNode> *with = null) {
    stmt_start();
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
        stmt->where = expression();
        if (stmt->where->dtype != D_BOOL) {
            show_error("WHERE clause must a boolean expression");
        }
    }

    if (peek()->type == T_GROUP) {
        pop();
        stmt->group = parse_group_by();
    }

    if (peek()->type == T_ORDER) {
        pop();
        stmt->order = parse_order_by(stmt->select);
    }

    if (peek()->type == T_LIMIT) {
        pop();
        stmt->limit = parse_limit();
    }

    validate_select(stmt->from, stmt->select);
    validate_where(stmt->from, stmt->where);
    if (!name.empty()) {
        // add to table map
        TABLES.insert({name, SELECTS});
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
        val temp = match(T_IDENTIFIER, "temporary table name")->text;
        match(T_AS, "as");
        match(T_LPAREN, "open parentheses");
        match(T_SELECT, "select clause");
        query = parse_select_stmt(temp);
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
    return parse_select_stmt("", with);
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
            stmt = parse_select_stmt("");
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
    TOKENS = tokens;
    init_std();
    return parse();
}
