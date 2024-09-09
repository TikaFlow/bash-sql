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
// maintain 3 lists for every select
// selected alias: <aliases, index>
static Map<String, int> ALIAS;
// columns selected
static Table *SELECTS;
// table used: <alias, origin table>
static Map<String, String> TABLES_USED;
// all tables
static Map<String, Table *> TABLES;
// init column number, 32 by default
static int COL_NUM;

static ASTNode *expression();

/**
 * peek one token
 * @param accept_eof true if return null is accepted
 * @return token, null if no more token
 */
static Token *peek(bool accept_eof = false) {
    static val size = TOKENS->size();
    if (INDEX >= size) {
        if (!accept_eof) {
            show_error("Unexpected end of sql");
        }
        return null;
    }
    return TOKENS->at(INDEX);
}

/**
 * pop one token
 * @param accept_eof true if return null is accepted
 * @return token, null if no more token
 */
static Token *pop(bool accept_eof = false) {
    Token *token = peek(accept_eof);
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
    Token *token = pop(true);
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
    TABLES_USED.clear();
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
    } else if (op_type >= A_EQ && op_type <= A_GE) { // binary logic operator
        if (left_type == D_BOOL || right_type == D_BOOL) {
            show_error("incompatible operands");
        }
        return D_BOOL;
    } else if (op_type >= A_AND && op_type <= A_OR) { // and/or operator
        if (left_type != D_BOOL || right_type != D_BOOL) {
            show_error("incompatible operands");
        }
        return D_BOOL;
    } else if (op_type >= A_NOT && op_type <= A_ISNULL) { // unary logic operator
        if (left_type == D_BOOL) {
            return D_BOOL;
        }
        show_error("incompatible operands");
    } else { // "like"
        return D_BOOL;
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
        do {
            param = expression();
            left = new ASTNode(A_PARAM, D_NONE, left, param);
        } while (pop()->type == T_COMMA);
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

    while (tk->type == T_STAR || tk->type == T_SLASH || tk->type == T_MOD) {
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

    while (tk->type == T_PLUS || tk->type == T_MINUS) {
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
 * @return ast node of the "in" list
 */
static ASTNode *in_list(ASTNode *exp) {
    match(T_LPAREN, "open parenthesis");
    // exp in (a, b) ==> exp = a or exp = b
    var left = new ASTNode(A_LITERAL, D_BOOL, null, null, false);

    do {
        var value = expression();
        if (value->atype != A_LITERAL) {
            show_error("Expected literal in list");
        }

        val right = new ASTNode(A_EQ, D_BOOL, exp, value);
        left = new ASTNode(A_OR, D_BOOL, left, right);
    } while (pop()->type == T_COMMA);
    unpop();

    match(T_RPAREN, "close parenthesis");
    return left;
}

/**
 * parse LIKE clause
 * @param exp expression to compare with
 * @return ast node of the LIKE clause
 */
static ASTNode *parse_like(ASTNode *exp) {
    val tk = match(T_STRING, "string literal");
    var value = new ASTNode(A_LITERAL, D_STRING, null, null, tk->text);

    return new ASTNode(A_LIKE, D_BOOL, exp, value);
}

/**
 * parse logical factor in expression
 * @note logical_factor ::= arithmetic_expression [comparison_operator arithmetic_expression]
                            | arithmetic_expression ["NOT"] ("IN" | "LIKE") in_list
                            | arithmetic_expression ["IS" ["NOT"] "NULL"]
        comparison_operator ::= ">" | "<" | ">=" | "<=" | "!=" | "<>"
 * @return ast node of the logical factor
 */
static ASTNode *logical_factor() {
    var left = arithmetic_expression();
    var tk = pop();
    if (tk->type >= T_EQ && tk->type <= T_GE) {
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
    } else if (tk->type == T_NOT || tk->type == T_IN || tk->type == T_LIKE) {

        val is_not = tk->type == T_NOT;
        if (is_not) {
            tk = pop(); // get the next token
        }

        if (tk->type == T_IN) {
            left = in_list(left);
        } else if (tk->type == T_LIKE) {
            left = parse_like(left);
        } else {
            show_error("Expected IN or LIKE after NOT");
        }

        if (is_not) {
            left = new ASTNode(A_NOT, D_BOOL, left, null);
        }
    } else if (tk->type == T_IS) {
        bool is_not = false;
        if (peek()->type == T_NOT) {
            pop();
            is_not = true;
        }
        match(T_NULL, "null");
        left = new ASTNode(A_ISNULL, D_BOOL, left, null);

        if (is_not) {
            left = new ASTNode(A_NOT, D_BOOL, left, null);
        }
    } else {
        unpop();
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

    while (pop()->type == T_AND) {
        val right = logical_factor();
        left = new ASTNode(A_AND, D_BOOL, left, right);
    }
    unpop();

    return left;
}

/**
 * parse logical expression
 * @note logical_expression ::= logical_term { "OR" logical_term }
 * @return ast node of the logical expression
 */
static ASTNode *logical_expression() {
    var left = logical_term();

    while (pop()->type == T_OR) {
        val right = logical_term();
        left = new ASTNode(A_OR, D_BOOL, left, right);
    }
    unpop();

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
 * @param i insert at index i
 * @param table table name
 * @return column count after expansion
 */
static int expand_star(Vector<SelectNode> *select, int i, const String &table) {
    int count = 0;
    var tables = Map<String, Table *>();

    if (table.empty()) { // *
        for (val &t: TABLES_USED) {
            tables.insert({t.first, TABLES[t.second]});
        }
    } else { // t.*
        tables.insert({table, TABLES[table]});
    }

    select->erase(select->begin() + i);
    for (auto &t: tables) {
        val cols = t.second;
        for (auto &col: *cols) {
            val name = col.first;
            val dtype = col.second;
            val node = new ASTNode(A_COLUMN, dtype, null, null, name);
            select->insert(select->begin() + i + count++, {node, name});
        }
    }

    return count;
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

    ALIAS.insert({name, 1}); // 1 is dummy
}

/**
 * parse SELECT clause
 * @return vector of SELECT node
 */
static Vector<SelectNode> *parse_select() {
    val select = new Vector<SelectNode>();
    do {
        String as;
        if (peek()->type == T_STAR) {
            pop();
            val col = new ASTNode(A_COLUMN, D_NONE, null, null, String("*"));
            // select *
            select->push_back({col, as});
            continue;
        }

        val col = expression();
        if (col->atype == A_COLUMN) {
            val col_name = resolve_column(col->s)->second;
            if (col_name != "*") {
                as = col_name;
            } else {
                // select t.*
                select->push_back({col, as});
                continue;
            }
        }
        if (peek()->type == T_AS) {
            pop();
            as = match(T_IDENTIFIER, "column alias")->text;
            add_alias(as);
        }

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
        TABLES_USED.insert({as.empty() ? name : as, name});
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

    TABLES_USED.insert({"std", "std"});
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
    if (node->atype < A_ADD || node->atype > A_ISNULL) {
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
        default:
            break;
    }

    release_node(&node->left);
    release_node(&node->right);
    node->atype = A_LITERAL; // now node itself is a literal
}

/**
 * optimize short circuit and node
 * @param node the ast root node
 */
static void circuit_and(ASTNode *node) {
    if (node->left->atype == A_LITERAL) {
        if (node->left->b) {
            node->atype = node->right->atype;
            node->b = node->right->b; // must be D_BOOL, so we copy b
            node->left = node->right->left;
            node->right = node->right->right;
            node->s = node->right->s;
        } else {
            node->atype = A_LITERAL;
            node->b = false;
        }
        release_node(&node->left);
        release_node(&node->right);
    } else if (node->right->atype == A_LITERAL) {
        if (node->right->b) {
            node->atype = node->left->atype;
            node->b = node->left->b;
            node->left = node->left->left;
            node->right = node->left->right;
            node->s = node->left->s;
        } else {
            node->atype = A_LITERAL;
            node->b = false;
        }
        release_node(&node->left);
        release_node(&node->right);
    }
}

/**
 * optimize short circuit or node
 * @param node the ast root node
 */
static void circuit_or(ASTNode *node) {
    if (node->left->atype == A_LITERAL) {
        if (node->left->b) {
            node->atype = A_LITERAL;
            node->b = true;
        } else {
            node->atype = node->right->atype;
            node->b = node->right->b;
            node->left = node->right->left;
            node->right = node->right->right;
            node->s = node->right->s;
        }
        release_node(&node->left);
        release_node(&node->right);
    } else if (node->right->atype == A_LITERAL) {
        if (node->right->b) {
            node->atype = A_LITERAL;
            node->b = true;
        } else {
            node->atype = node->left->atype;
            node->b = node->left->b;
            node->left = node->left->left;
            node->right = node->left->right;
            node->s = node->left->s;
        }
        release_node(&node->left);
        release_node(&node->right);
    }
}

/**
 * optimize short circuit
 * @param node the ast root node
 */
static void short_circuit(ASTNode *node) {
    if (node->atype < A_AND || node->atype > A_OR) {
        return;
    }

    if (node->atype == A_AND) {
        circuit_and(node);
    }
    if (node->atype == A_OR) {
        circuit_or(node);
    }
}

/**
 * optimize the ast
 * @param node the ast root node
 */
static void optimize_ast(ASTNode *node) {
    constant_fold(node);
    short_circuit(node);
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
    } else if (node->atype == A_PARAM) { // if it is a param
        node->dtype = node->right->dtype;
    } else if (node->atype >= A_ADD) {
        node->dtype = repair_type(node->left, node->right, node->atype);
    }

    // optimize the ast
    optimize_ast(node);
}

/**
 * check if the select columns are valid
 * @param from FROM table
 * @param select SELECT clause
 */
static void validate_select(Map<String, ColumnDesc> *from, Vector<SelectNode> *select) {
    // assert(select != null); // select would never be null
    for (int i = 0; i < select->size(); i++) {
        val &col = select->at(i).col;

        if (col->atype == A_COLUMN && resolve_column(col->s)->second == "*") {
            val count = expand_star(select, i, resolve_column(col->s)->first);
            i += count - 1;
        } else {
            check_ast(from, col);
        }
    }

    // then fix the ALIAS map and SELECT list
    ALIAS.clear();
    int index = 0;
    for (val &col_exp: *select) {
        var name = col_exp.as;
        if (!name.empty()) {
            ALIAS[name] = index;
        } else if (col_exp.col->atype == A_COLUMN) {
            name = resolve_column(col_exp.col->s)->second;
        }

        // dtype has been repaired
        SELECTS->emplace_back(name, col_exp.col->dtype);

        index++;
    }
}

/**
 * check if the WHERE columns are valid
 * @param from FROM table
 * @param where WHERE clause
 */
static void validate_where(Map<String, ColumnDesc> *from, ASTNode *where) {
    if (!where) {
        return;
    }
    check_ast(from, where);
    if (where->dtype != D_BOOL) {
        show_error("WHERE clause must a boolean expression");
    }
}

/**
 * check if the group by columns are valid
 * @param from FROM table
 * @param select SELECT clause
 * @param group GROUP BY clause
 */
static void validate_group_by(Map<String, ColumnDesc> *from, Vector<SelectNode> *select, Vector<ASTNode *> *group) {
    if (!group) {
        return;
    }
    for (val &col: *group) {
        if (col->atype == A_COLUMN) {
            check_ast(from, col);
        } else if (col->atype == A_LITERAL && col->dtype == D_INT && col->l > 0 && col->l < select->size()) {
            val column = select->at(col->l - 1).col;
            if (column->atype == A_COLUMN) {
                col->atype = column->atype;
                col->dtype = column->dtype;
                col->s = column->s;
            } else {
                show_error("Group by clause must be a column");
            }
        } else {
            show_error("Group by clause must be a column or a position in select clause");
        }
    }
}

/**
 * check if the order by columns are valid
 * @param order ORDER BY clause
 */
static void validate_order_by(Vector<OrderNode> *order) {
    if (!order) {
        return;
    }
    for (val &col: *order) {
        if (col.index < 0) {
            show_error("Column index must be greater than 0");
        }
    }
}

/**
 * check if the statement is valid
 * @param stmt the statement to be checked
 * @param after_where if we want to check clause after WHERE
 */
static void validate_stmt(SelectStatement *stmt, bool after_where = false) {
    if (after_where) {
        // validate group_by and order_by
        validate_group_by(stmt->from, stmt->select, stmt->group);
        validate_order_by(stmt->order);
        return;
    }
    validate_select(stmt->from, stmt->select);
    validate_where(stmt->from, stmt->where);
}

/**
 * parse GROUP BY clause
 * @return vector of GROUP BY column expression
 */
static Vector<ASTNode *> *parse_group_by() {
    val node = new Vector<ASTNode *>();
    match(T_BY, "by");

    do {
        val col = expression();
        node->push_back(col);
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
        } else if (token->type == T_IDENTIFIER) {
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

    validate_stmt(stmt);

    if (peek()->type == T_GROUP) {
        pop();
        stmt->group = parse_group_by();
    }

    if (peek()->type == T_ORDER) {
        pop();
        stmt->order = parse_order_by(stmt->select);
    }

    validate_stmt(stmt, true);

    if (peek()->type == T_LIMIT) {
        pop();
        stmt->limit = parse_limit();
    }

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
    while ((start = pop(true))) {
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
Vector<SelectStatement *> *parse(Vector<Token *> *tokens, int col_count) {
    TOKENS = tokens;
    COL_NUM = col_count;
    init_std();
    return parse();
}
