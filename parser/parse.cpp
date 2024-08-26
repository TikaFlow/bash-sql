//
// Created by tika on 24-8-14.
//

#include "parse.h"

// next token index to be process
static int INDEX = 0;
// token vector from lexer
static const Vector<Token *> *TOKENS;
// build-in functions, <name, type>
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
 * cast a string literal node to a number node
 * @param node node to be cast
 */
static void cast_string(ASTNode *node) {
    if (node->atype != A_LITERAL) {
        return;
    }

    val &str = node->s;
    if (is_integer(str)) {
        val value = stol(str);
        node->l = value;
        node->dtype = D_INT;
    } else if (is_double(str)) {
        val value = stod(str);
        node->d = value;
        node->dtype = D_REAL;
    } else {
        show_error("cannot convert string to number: " + str);
    }
}

/**
 * compatible data type of two sides
 * @param left left data type
 * @param right right data type
 * @return compatible data type
 */
static DataType repair_type(ASTNode *left, ASTNode *right, ASTType op_type) {
    if (op_type >= A_ADD) {
        if (!left) {
            show_error("missing required operands");
        }
        if (op_type <= A_OR && !right) {
            show_error("missing required operands");
        }
    }

    DataType left_type = left->dtype, right_type = right ? right->dtype : D_NONE;

    if (op_type == A_NEGATE || (op_type >= A_ADD && op_type <= A_MOD)) { // mathematical operator
        // if one of them is string literal, then we try to convert it to integer/real
        if (left_type == D_STRING) {
            cast_string(left);
        }
        if (right_type == D_STRING) {
            cast_string(right);
        }

        // get type again because they may have been changed
        left_type = left->dtype, right_type = right ? right->dtype : D_NONE;

        if (op_type == A_NEGATE) {
            if (left_type == D_BOOL) {
                show_error("incompatible operands of negate");
            }

            return left_type;
        }

        if (left_type == D_BOOL || right_type == D_BOOL) {
            show_error("incompatible operands");
        }

        if (op_type == A_MOD) {
            if (left_type != D_INT || right_type != D_INT) {
                show_error("incompatible operands of modulo operator");
            }
        }

        // if there is a determined D_REAL, then we get D_REAL
        if (left_type == D_REAL || right_type == D_REAL) {
            return D_REAL;
        }

        // if both are D_INT, then we get D_INT
        if (left_type == D_INT && right_type == D_INT) {
            return D_INT;
        }

        return D_NUMBER;
    } else if (op_type >= A_EQ && op_type <= A_OR) { // binary logic operator
        if (left_type == D_BOOL || right_type == D_BOOL) {
            show_error("incompatible operands");
        }
        return D_BOOL;
    } else if (op_type >= A_NOT && op_type <= A_NOTNULL) { // unary logic operator
        if (left_type == D_BOOL) {
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
static Pair<String, String> *resolve_column(const String &str) {
    val pos = str.find('.');

    if (pos == npos) {
        return new Pair<String, String>{"", str};
    }
    return new Pair<String, String>{str.substr(0, pos), str.substr(pos + 1)};
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

    var tk = pop();
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
            name = tk->text;
            if (peek()->type == T_LPAREN) {
                node = function_call(name);
            } else {
                node = new ASTNode(A_COLUMN, D_NONE, null, null, name);
            }
            break;
        case T_LPAREN:
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
 * @note factor ::= ["-"] primary | "NOT" primary
 * @return ast node of the factor
 */
static ASTNode *factor() {
    val tk = peek();
    if (tk->type == T_MINUS || tk->type == T_NOT) {
        pop();
    }
    var left = primary();

    if (tk->type == T_MINUS) {
        left = new ASTNode(A_NEGATE, D_NUMBER, left, null);
    } else if (tk->type == T_NOT) {
        left = new ASTNode(A_NOT, D_BOOL, left, null);
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
        left = new ASTNode(atype, D_NUMBER, left, right);
        tk = peek();
    }

    return left;
}

/**
 * parse arithmetic expression
 * @note arithmetic_expression ::= term { ( "+" | "-" ) term }
 * @return ast node of the arithmetic expression
 */
static ASTNode *arithmetic_expression() {
    var left = term();
    var tk = peek();

    while (tk && (tk->type == T_PLUS || tk->type == T_MINUS)) {
        pop();
        val right = term();
        val atype = tk->type == T_PLUS ? A_ADD : A_SUB;
        left = new ASTNode(atype, D_NUMBER, left, right);
        tk = peek();
    }

    return left;
}

/**
 * parse list of expressions, every expression should be literal
 * @note in_list ::= expression_list ::= "(" expression {"," expression} ")"
 * @param exp expression to compare with
 * @param is_in true if it's "in", false if it's "not in"
 * @return ast node of the "in" list
 */
static ASTNode *in_list(ASTNode *exp, bool is_in) {
    match(T_LPAREN, "open parenthesis");
    // exp in (a, b) ==> exp = a or exp = b
    // exp not in (a, b) ==> exp != a and exp != b
    val atype1 = is_in ? A_OR : A_AND;
    val atype2 = is_in ? A_EQ : A_NE;

    var value = expression();
    if (value->atype != A_LITERAL) {
        show_error("Expected literal list");
    }

    var left = new ASTNode(atype2, D_BOOL, exp, value);
    var tk = peek();

    while (tk && tk->type == T_COMMA) {
        pop();

        value = expression();
        if (value->atype != A_LITERAL) {
            show_error("Expected literal list");
        }

        val right = new ASTNode(atype2, D_BOOL, exp, value);
        left = new ASTNode(atype1, D_BOOL, left, right);
        tk = peek();
    }

    match(T_RPAREN, "close parenthesis");
    return left;
}

/**
 * parse logical factor in expression
 * @note logical_factor ::= arithmetic_expression [comparison_operator arithmetic_expression]
                            | arithmetic_expression ["NOT"] "IN" in_list
                            | arithmetic_expression ["IS" ["NOT"] "NULL"]
        comparison_operator ::= ">" | "<" | ">=" | "<=" | "!=" | "<>"
 * @return ast node of the logical factor
 */
static ASTNode *logical_factor() {
    var left = arithmetic_expression();
    var tk = peek();
    if (tk->type >= T_EQ && tk->type <= T_GE) {
        pop();
        val right = arithmetic_expression();
        ASTType atype;
        switch (tk->type) {
            case T_EQ:
                atype = A_EQ;
                break;
            case T_NE1:
            case T_NE2:
                atype = A_NE;
                break;
            case T_LT:
                atype = A_LT;
                break;
            case T_GT:
                atype = A_GT;
                break;
            case T_LE:
                atype = A_LE;
                break;
            case T_GE:
                atype = A_GE;
                break;
            default:
                break;
        }

        left = new ASTNode(atype, D_BOOL, left, right);
    } else if (tk->type == T_NOT || tk->type == T_IN) {
        pop();

        val is_in = tk->type == T_IN;
        if (!is_in) {
            match(T_IN, "in");
        }

        left = in_list(left, is_in);
    } else if (tk->type == T_IS) {
        pop();

        var atype = A_ISNULL;
        if (peek()->type == T_NOT) {
            pop();
            atype = A_NOTNULL;
        }

        match(T_NULL, "null");

        left = new ASTNode(atype, D_BOOL, left, null);
    }

    return left;
}

/**
 * parse logical term in expression
 * @note logical_term ::= logical_factor { "AND" logical_factor }
 * @return ast node of the logical term
 */
static ASTNode *logical_term() {
    var left = logical_factor();
    var tk = peek();

    while (tk && tk->type == T_AND) {
        pop();
        val right = logical_factor();
        left = new ASTNode(A_AND, D_BOOL, left, right);
        tk = peek();
    }

    return left;
}

/**
 * parse logical expression
 * @note logical_expression ::= logical_term { "OR" logical_term }
 * @return ast node of the logical expression
 */
static ASTNode *logical_expression() {
    var left = logical_term();
    var tk = peek();

    while (tk && tk->type == T_OR) {
        pop();
        val right = logical_term();
        left = new ASTNode(A_OR, D_BOOL, left, right);
        tk = peek();
    }

    return left;
}

/**
 * parse column expression in select clause
 * @note expression ::= logical_expression
 * @return ast node of the column expression
 */
static ASTNode *expression() {
    return logical_expression();
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
            expand_star(select); // we cannot get all columns now, so we need to expand it later
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
            from->insert({as + (as.empty() ? "" : ".") + col.first, {col.second, index}});
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
 * release the memory of AST node
 * @param node the node to release
 */
static inline void release_node(ASTNode **node) {
    if (*node) {
        val temp = *node;
        *node = null;
        delete temp;
    }
}

/**
 * fold constant expression
 * @param node the node to fold
 */
static void constant_fold(ASTNode *node) {
    if (node->atype < A_ADD) {
        return;
    }

    if (node->left->atype != A_LITERAL) {
        return;
    }

    if (node->atype <= A_OR && node->right->atype != A_LITERAL) {
        return;
    }

    // till now, left and right(if there has) are both literal, and string literal has been converted
    double lv = node->left->dtype == D_INT ? (double) node->left->l : node->left->d;
    double rv = node->right->dtype == D_INT ? (double) node->right->l : node->right->d;
    long ll = node->left->l;
    long rl = node->right->l;

    long dl = 1;
    double dd = 1.0;
    if (node->dtype == D_INT) {
        dl = node->left->l - node->right->l;
    } else {
        dd = lv - rv;
    }
    dd = (double) dl * dd;
    switch (node->atype) {
        /*
         * A_ADD, A_SUB, A_MUL, A_DIV, A_MOD,
         * A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
         * A_NEGATE, A_NOT, A_ISNULL, A_NOTNULL,
         */
        case A_ADD:
            if (node->dtype == D_INT) {
                node->l = node->left->l + node->right->l;
            } else {
                node->d = lv + rv;
            }
            break;
        case A_SUB:
            if (node->dtype == D_INT) {
                node->l = node->left->l - node->right->l;
            } else {
                node->d = lv - rv;
            }
            break;
        case A_MUL:
            if (node->dtype == D_INT) {
                node->l = node->left->l * node->right->l;
            } else {
                node->d = lv * rv;
            }
            break;
        case A_DIV:
            if (node->dtype == D_INT) {
                node->l = node->left->l / node->right->l;
            } else {
                node->d = lv / rv;
            }
            break;
        case A_MOD:
            node->l = node->left->l % node->right->l;
            break;
        case A_EQ:
            node->b = dd == 0;
            break;
        case A_NE:
            node->b = dd != 0;
            break;
        case A_LT:
            node->b = dd < 0;
            break;
        case A_GT:
            node->b = dd > 0;
            break;
        case A_LE:
            node->b = dd <= 0;
            break;
        case A_GE:
            node->b = dd >= 0;
            break;
        case A_AND:
            node->b = node->left->b && node->right->b;
            break;
        case A_OR:
            node->b = node->left->b || node->right->b;
            break;
        case A_NEGATE:
            if (node->dtype == D_INT) {
                node->l = -node->left->l;
            } else { // then must be D_REAL
                node->d = -node->left->d;
            }
            break;
        case A_NOT:
            node->b = !node->left->b;
            break;
        case A_ISNULL:
            // only string literal can be null
            node->b = node->left->dtype == D_STRING && node->left->s.empty();
            break;
        case A_NOTNULL:
            node->b = node->left->dtype != D_STRING || !node->left->s.empty();
            break;
        default:
            break;
    }

    release_node(&node->left);
    release_node(&node->right);
    node->atype = A_LITERAL; // now node itself is a literal
}

/**
 * check if the column is valid
 * and repair types if needed
 * @param from FROM table
 * @param node the node containing the column
 */
static void check_ast(Map<String, ColumnDesc> *from, ASTNode *node) {
    if (!node) {
        return;
    }

    if (node->left) {
        check_ast(from, node->left);
    }
    if (node->right) {
        check_ast(from, node->right);
    }

    if (node->atype == A_COLUMN) { // if it is a column
        if (!from->count(node->s)) {
            show_error("Column not found: " + node->s);
        }
        node->dtype = from->at(node->s).first; // repair type
    } else if (node->atype >= A_ADD) {
        node->dtype = repair_type(node->left, node->right, node->atype);
    }

    // optimize the ast
    constant_fold(node);
}

/**
 * check if the select columns are valid
 * @param from FROM table
 * @param select SELECT clause
 */
static void validate_select(Map<String, ColumnDesc> *from, Vector<SelectNode> *select) {
    // expand *
    // assert(select != null); // select would never be null
    for (val &node: *select) {
        check_ast(from, node.col);
    }
}

/**
 * check if the WHERE columns are valid
 * @param from FROM table
 * @param where WHERE clause
 */
static void validate_where(Map<String, ColumnDesc> *from, ASTNode *where) {
    check_ast(from, where);
    if (where->dtype != D_BOOL) {
        show_error("WHERE clause must a boolean expression");
    }
}

/**
 * check if the statement is valid
 * @param stmt the statement to be checked
 */
static void validate_stmt(SelectStatement *stmt) {
    validate_select(stmt->from, stmt->select);
    validate_where(stmt->from, stmt->where);
    // validate group_by? I don't know how to do that yet
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

    validate_stmt(stmt);
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
