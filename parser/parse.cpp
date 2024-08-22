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

static ASTNode *expression();

/**
 * compatible data type of two sides
 * @param left left data type
 * @param right right data type
 * @return compatible data type
 */
static DataType cast_type(DataType left, DataType right) {
    if (left == D_STRING || right == D_STRING || left == D_BOOL || right == D_BOOL) {
        show_error("illegal operands");
    }

    if (left == D_REAL || right == D_REAL) {
        return D_REAL;
    }

    return D_INT;
}

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
 * parse function call
 * @note function_call ::= identifier "(" [expression {"," expression}] ")"
 * @param name function name
 * @return ast node of the function call
 */
static ASTNode *function_call(const String &name) {
    ASTNode *node, *left = null;

    pop(); // pop "("
    if (peek()->type != T_RPAREN) {
        left = expression();
        var tk = peek();
        while (tk->type == T_COMMA) {
            pop();
            // todo get type
            left = new ASTNode(A_PARAM, D_STRING, left, null);
        }
    }
    match(T_RPAREN, "close parenthesis");
    // todo check if the function exists and get the return type
    node = new ASTNode(A_FUNC_CALL, D_STRING, left, null);

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
            node = new ASTNode(A_LITERAL, D_INT, null, null);
            break;
        case T_REAL:
            node = new ASTNode(A_LITERAL, D_REAL, null, null);
            break;
        case T_STRING:
            node = new ASTNode(A_LITERAL, D_STRING, null, null);
            break;
        case T_IDENTIFIER:
            name = pop()->text;
            if (peek()->type == T_LPAREN) {
                node = function_call(name);
            } else {
                // todo check if the column exists and get the type
                node = new ASTNode(A_COLUMN, D_STRING, null, null);
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
        val dtype = cast_type(left->dtype, right->dtype);
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
        val dtype = cast_type(left->dtype, right->dtype);
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
        if (peek()->type == T_AS) {
            pop();
            as = match(T_STRING, "column alias")->text;
            // add alias to list to avoid conflict
        }

        select->push_back({col, as});
    } while (pop()->type == T_COMMA);
    unpop();

    return select;
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
        val temp = match(T_IDENTIFIER, "temporary table name")->text;
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
