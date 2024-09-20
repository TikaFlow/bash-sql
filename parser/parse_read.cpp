//
// Created by tika on 24-8-14.
//

#include "util.h"
#include "funcs.h"
#include "parser.h"

bool ASTNode::tableless() const {
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

bool SelectStatement::tableless_with() const {
    if (!with) {
        return true;
    }

    return all_of(with->begin(), with->end(), [](WithNode &node) { return node.stmt->tableless(); });
}

bool SelectStatement::tableless_select() const {
    if (!select) {
        return true;
    }

    return all_of(select->begin(), select->end(), [](SelectNode &node) { return node.col->tableless(); });
}

bool SelectStatement::tableless() const {
    return tableless_with() && tableless_select();
}

// next token index to be process
static int INDEX = 0;
// token vector from lexer
static const Vector<Token *> *TOKENS;
// maintain 3 lists for every select
// selected alias: <aliases, index>
static Map<String, int> ALIAS;
// columns selected
static Schema *SELECTS;
// table used: <alias, origin table>
static Map<String, String> TABLES_USED;
// all tables
static Map<String, Schema *> TABLES;

static ASTNode *expression();

/**
 * peek one token
 * @param accept_eof true if return null is accepted
 * @return token, null if no more token
 */
static Token *peek(bool accept_eof = false) {
    if (INDEX >= TOKENS->size()) {
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
        show_error("Expected " + what + " but got " + (token ? token->text : "nothing"));
    }
    return null; // make compiler happy
}

/**
 * initialize the std table
 */
static void init_parser(Vector<Token *> *tokens) {
    TOKENS = tokens;
    INDEX = 0;
    TABLES.clear();

    for (val &table: *db) {
        TABLES.insert({table.first, table.second.first});
    }
}

/**
 * prepare the table for parsing
 */
static void stmt_start() {
    ALIAS.clear();
    SELECTS = new Schema();
    TABLES_USED.clear();
}

/**
 * cast a string literal node to a number node
 * @param node node to be cast
 * @return true if cast successfully
 */
static bool cast_string(ASTNode *node) {
    val &str = node->text;
    if (is_integer(str)) {
        node->number = stoi(str);
        node->dtype = D_INTEGER;
    } else if (is_double(str)) {
        node->number = stod(str);
        node->dtype = D_REAL;
    } else {
        return false;
    }

    return true;
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

    var l_nan = false, r_nan = false;
    // if one of them is string literal, then we try to convert it to integer/real
    if (left->atype == A_LITERAL && left->dtype == D_STRING) {
        l_nan = !cast_string(left);
    }
    if (right && right->atype == A_LITERAL && right->dtype == D_STRING) {
        r_nan = !cast_string(left);
    }
    val left_type = left->dtype, right_type = right ? right->dtype : D_NONE;

    if (op_type == A_NEGATE || (op_type >= A_ADD && op_type <= A_DIV)) { // mathematical operator
        if (op_type == A_NEGATE) {
            if (l_nan) {
                show_error("cannot convert string to number: " + left->text);
            }
            if (left_type == D_BOOL) {
                show_error("incompatible operands of negate");
            }

            return left_type;
        }

        if (l_nan || r_nan) {
            show_error("cannot convert string to number: " + (l_nan ? left->text : right->text));
        }

        if (left_type == D_BOOL || right_type == D_BOOL) {
            show_error("incompatible operands");
        }

        // if there is a determined D_REAL, then we get D_REAL
        if (left_type == D_REAL || right_type == D_REAL) {
            return D_REAL;
        }

        // if both are D_INT, then we get D_INT
        if (left_type == D_INTEGER && right_type == D_INTEGER) {
            return D_INTEGER;
        }

        return D_NUMBER;
    } else if (op_type >= A_EQ && op_type <= A_GE) { // binary logic operator
        if (left_type == D_BOOL || right_type == D_BOOL) {
            show_error("incompatible operands for '==' or '!='");
        }
        return D_BOOL;
    } else if (op_type >= A_AND && op_type <= A_OR) { // and/or operator
        if (left_type != D_BOOL || right_type != D_BOOL) {
            show_error("incompatible operands for 'AND' or 'OR'");
        }
        return D_BOOL;
    } else if (op_type == A_NOT) { // unary logic operator
        if (left_type == D_BOOL) {
            return D_BOOL;
        }
        show_error("incompatible operands for 'NOT'");
    } else { // "is_null"|"like"
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
    ASTNode *left = null, *param;
    val func = to_lower(name);
    if (!FUNCTIONS.count(func)) {
        show_error("Unknown function: '" + func + "'");
    }

    pop(); // pop "("
    if (peek()->type != T_RPAREN) {
        do {
            param = expression();
            left = new ASTNode(A_PARAM, D_NONE, left, param);
        } while (pop()->type == T_COMMA);
        unpop();
    }
    match(T_RPAREN, "close parenthesis");
    return new ASTNode(A_FUNC_CALL, FUNCTIONS.at(func).dtype, left, null, func);
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
            node = new ASTNode(A_LITERAL, D_INTEGER, null, null, tk->integer);
            break;
        case T_REAL:
            node = new ASTNode(A_LITERAL, D_REAL, null, null, tk->real);
            break;
        case T_TRUE:
        case T_FALSE:
            node = new ASTNode(A_LITERAL, D_BOOL, null, null, tk->type == T_TRUE);
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

    while (tk->type == T_STAR || tk->type == T_SLASH) {
        pop();
        val right = factor();
        val atype = tk->type == T_STAR ? A_MUL : A_DIV;
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
    var tables = Map<String, Schema *>();

    if (table.empty()) { // *
        for (val &t: TABLES_USED) {
            tables.insert({t.first, TABLES[t.second]});
        }
    } else { // t.*
        tables.insert({table, TABLES[table]});
    }

    select->erase(select->begin() + i);
    for (val &t: tables) {
        val cols = t.second;
        for (val &col: *cols) {
            val name = t.first + "." + col.first;
            val dtype = col.second;
            val node = new ASTNode(A_COLUMN, dtype, null, null, name);
            select->insert(select->begin() + i + count++, {node, ""});
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
 * release the memory of AST node
 * @param node the node to release
 */
static inline void release_node(ASTNode *&node) {
    if (!node) {
        return;
    }

    if (node->left) {
        release_node(node->left);
    }
    if (node->right) {
        release_node(node->right);
    }

    delete node;
    node = null;
}

/**
 * fold constant expression
 * @param node the node to fold
 */
static void constant_fold(ASTNode *node) {
    if (node->atype < A_ADD || node->atype > A_ISNULL || node->left->atype != A_LITERAL) {
        return;
    }

    if (node->atype <= A_OR && node->right->atype != A_LITERAL) {
        return;
    }
    // till now, left and right(if there has) are both literal

    var left = node->left, right = node->right;
    val cmp = compare_number(left->number, right ? right->number : 0);
    switch (node->atype) { // never be A_LIKE
        /*
         * A_ADD, A_SUB, A_MUL, A_DIV,
         * A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_AND, A_OR,
         * A_NEGATE, A_NOT, A_ISNULL,
         */
        case A_ADD:
            node->number = left->number + right->number;
            break;
        case A_SUB:
            node->number = left->number - right->number;
            break;
        case A_MUL:
            node->number = left->number * right->number;
            break;
        case A_DIV:
            node->number = left->number / right->number;
            break;
        case A_EQ:
            if (left->dtype != D_STRING) {
                left->text = to_string(left->number);
            }
            if (right->dtype != D_STRING) {
                right->text = to_string(right->number);
            }
            // just compare their text
            node->number = left->text == right->text;
            break;
        case A_NE:
            if (left->dtype != D_STRING) {
                left->text = to_string(left->number);
            }
            if (right->dtype != D_STRING) {
                right->text = to_string(right->number);
            }
            node->number = left->text != right->text;
            break;
        case A_LT:
            node->number = cmp < 0;
            break;
        case A_GT:
            node->number = cmp > 0;
            break;
        case A_LE:
            node->number = cmp <= 0;
            break;
        case A_GE:
            node->number = cmp >= 0;
            break;
        case A_AND:
            node->number = (bool) left->number && (bool) right->number;
            break;
        case A_OR:
            node->number = (bool) left->number || (bool) right->number;
            break;
        case A_NEGATE:
            node->number = -left->number;
            break;
        case A_NOT:
            node->number = left->number == 0;
            break;
        case A_ISNULL:
            // if a literal, only string can be null
            node->number = left->dtype == D_STRING && left->text.empty();
            break;
        default:
            break; // make compiler happy
    }

    release_node(node->left);
    release_node(node->right);
    node->atype = A_LITERAL; // now node itself is a literal
}

/**
 * optimize short circuit and node
 * @param node the ast root node
 */
static void circuit_and(ASTNode *node) {
    if (node->left->atype == A_LITERAL) {
        if ((bool) node->left->number) {
            node->atype = node->right->atype;
            node->left = node->right->left;
            node->right = node->right->right;
            node->number = node->right->number;
            node->text = node->right->text;
        } else {
            node->atype = A_LITERAL;
            node->number = false;
        }
    } else if (node->right->atype == A_LITERAL) {
        if ((bool) node->right->number) {
            node->atype = node->left->atype;
            node->left = node->left->left;
            node->right = node->left->right;
            node->number = node->left->number;
            node->text = node->left->text;
        } else {
            node->atype = A_LITERAL;
            node->number = false;
        }
    }
}

/**
 * optimize short circuit or node
 * @param node the ast root node
 */
static void circuit_or(ASTNode *node) {
    if (node->left->atype == A_LITERAL) {
        if ((bool) node->left->number) {
            node->atype = A_LITERAL;
            node->number = true;
        } else {
            node->atype = node->right->atype;
            node->left = node->right->left;
            node->right = node->right->right;
            node->number = node->right->number;
            node->text = node->right->text;
        }
    } else if (node->right->atype == A_LITERAL) {
        if ((bool) node->right->number) {
            node->atype = A_LITERAL;
            node->number = true;
        } else {
            node->atype = node->left->atype;
            node->left = node->left->left;
            node->right = node->left->right;
            node->number = node->left->number;
            node->text = node->left->text;
        }
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

    /*
     * literal-literal: processed in constant_fold
     * !literal-!literal: return
     * !literal-literal: go on
     * literal-!literal: go on
     */
    if (node->left->atype != A_LITERAL && node->right->atype != A_LITERAL) {
        return;
    }

    if (node->atype == A_AND) {
        circuit_and(node);
    } else {
        circuit_or(node);
    }
    release_node(node->left);
    release_node(node->right);
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
 * check if has aggregate functions
 * @param node the ast root node
 * @return true if has
 */
static bool has_aggregate(ASTNode *node) {
    if (!node) {
        return false;
    }

    if (node->atype != A_FUNC_CALL) {
        return has_aggregate(node->left) || has_aggregate(node->right);
    }

    // if a func_call node
    val ftype = FUNCTIONS.at(node->text).ftype;
    if (ftype == F_AGGREGATE) {
        return true;
    }

    var param = node->left;
    while (param) {
        if (has_aggregate(param->right)) {
            return true;
        }
        param = param->left;
    }
    return false;
}

/**
 * check if has random functions
 * @param node the ast root node
 * @return true if has
 */
static bool has_random(ASTNode *node) {
    if (!node) {
        return false;
    }

    if (node->atype == A_FUNC_CALL && node->text == "rand") {
        return true;
    }

    return has_random(node->left) || has_random(node->right);
}

/**
 * check if the column is valid
 * and repair types if needed
 * @param from FROM table
 * @param node the node containing the column
 */
static void check_ast(TableSet *from, ASTNode *node) {
    if (!node) {
        return;
    }

    if (has_aggregate(node) && has_random(node)) {
        show_error("Aggregate functions cannot use with random functions");
    }

    if (node->left) {
        check_ast(from, node->left);
    }
    if (node->right) {
        check_ast(from, node->right);
    }

    if (node->atype == A_COLUMN) { // if it is a column
        if (!from->count(node->text)) {
            show_error("Column not found: " + node->text);
        }
        val desc = from->at(node->text);
        node->dtype = desc.first; // repair type
        if (desc.second != -1) {
            node->number = desc.second; // save col index
        } else {
            show_error("Ambiguous column: " + node->text);
        }
    } else if (node->atype == A_PARAM) { // if it is a param
        node->dtype = node->right->dtype;
    } else if (node->atype >= A_ADD) {
        node->dtype = repair_type(node->left, node->right, node->atype);
    }

    // optimize the ast
    optimize_ast(node);
}

/**
 * check if has nested aggregate functions
 * @param node the ast root node
 * @return true if has
 */
static bool has_nested_aggregate(ASTNode *node) {
    if (!node) {
        return false;
    }

    if (node->atype != A_FUNC_CALL) {
        return has_nested_aggregate(node->left) || has_nested_aggregate(node->right);
    }

    // if a func_call node
    var param = node->left;
    val ftype = FUNCTIONS.at(node->text).ftype;
    while (param) {
        if ((ftype != F_AGGREGATE && has_nested_aggregate(param->right))
            || (ftype == F_AGGREGATE && has_aggregate(param->right))) {
            return true;
        }
        param = param->left;
    }
    return false;
}

/**
 * check if there is column outside of aggregate function but not in group by
 * @param node the ast root node
 * @param group the group by clause
 * @return true if has
 */
static bool outside_agg(ASTNode *node, Vector<ASTNode *> *&group, ASTNode *&who) {
    if (!node) {
        return false;
    }

    if (node->atype == A_COLUMN) {
        val res = none_of(group->begin(), group->end(), [&](ASTNode *gnode) { return gnode->number == node->number; });
        if (res) {
            who = node;
            return true;
        }
    }

    if (node->atype != A_FUNC_CALL) {
        return outside_agg(node->left, group, who) || outside_agg(node->right, group, who);
    }

    // if a func_call node
    val ftype = FUNCTIONS.at(node->text).ftype;
    if (ftype == F_AGGREGATE) {
        return false;
    }

    var param = node->left;
    while (param) {
        if (outside_agg(param->right, group, who)) {
            return true;
        }
        param = param->left;
    }

    return false;
}

/**
 * check if the select columns are valid
 * @param from FROM table
 * @param select SELECT clause
 */
static void validate_select(TableSet *from, Vector<SelectNode> *select) {
    // assert(select != null); // select would never be null
    for (var i = 0; i < select->size(); i++) {
        val &col = select->at(i).col;

        if (col->atype == A_COLUMN && resolve_column(col->text)->second == "*") {
            val count = expand_star(select, i, resolve_column(col->text)->first);
            i += count - 1;
        } else {
            // check if there are nested aggregate functions
            if (has_nested_aggregate(col)) {
                show_error("Nested aggregate functions are not allowed");
            }
        }
    }

    // then fix the ALIAS map and SELECT list
    ALIAS.clear();
    int index = 0;
    for (var &col_exp: *select) {
        check_ast(from, col_exp.col);
        var name = col_exp.as;
        if (!name.empty()) {
            ALIAS[name] = index;
        } else if (col_exp.col->atype == A_COLUMN) {
            name = resolve_column(col_exp.col->text)->second;
            col_exp.as = name;
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
static void validate_where(TableSet *from, ASTNode *where) {
    if (!where) {
        return;
    }

    if (has_aggregate(where) || has_random(where)) {
        show_error("Aggregate or random functions are not allowed in condition expression");
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
static void validate_group_by(TableSet *from, Vector<SelectNode> *select, Vector<ASTNode *> *&group) {
    if (!group) {
        if (all_of(select->begin(), select->end(), [](const SelectNode &node) {
            return node.col->atype == A_LITERAL || has_aggregate(node.col);
        })) {
            // special case: we should have a group even if there is no GROUP BY clause
            group = new Vector<ASTNode *>();
            group->emplace_back(new ASTNode(A_COLUMN, D_NONE, null, null, -1));
        } else if (!none_of(select->begin(), select->end(), [](const SelectNode &node) {
            return has_aggregate(node.col);
        })) {
            show_error("Column without aggregate function must be in group by clause");
        }
        return;
    }

    var columns = Map<double, String>();

    for (val &col: *group) {
        if (col->atype == A_COLUMN) {
            check_ast(from, col);
        } else if (col->atype == A_LITERAL && col->dtype == D_INTEGER
                   && col->number > 0 && (int) col->number < select->size()) {
            val column = select->at((int) col->number - 1).col;
            if (column->atype == A_COLUMN || column->atype == A_LITERAL) {
                col->atype = column->atype;
                col->dtype = column->dtype;
                col->number = column->number;
                col->text = column->text;
            } else {
                show_error("Group index must refer tp a column or literal: " + to_string(col->number));
            }
        } else {
            show_error("Group by clause must be a column or a position in select clause");
        }

        if (col->atype == A_COLUMN) {
            columns.insert({col->number, col->text});
        }
    }

    // check if select columns are valid
    for (val &node: *select) {
        val &col = node.col;
        if (col->atype == A_LITERAL) { // literal is always valid
            continue;
        }

        val has_agg = has_aggregate(col);
        if (has_agg) {
            var who = col;
            if (outside_agg(col, group, who)) {
                show_error("Column outside aggregate function must be in group by clause: " + who->text);
            }
        } else {
            if (col->atype != A_COLUMN
                || none_of(group->begin(), group->end(), [&](ASTNode *gnode) {
                return gnode->number == col->number;
            })) {
                show_error("Column without aggregate function must be in group by clause: " + col->text);
            }

            columns.erase(col->number);
        }
    }

    if (!columns.empty()) {
        var cols = columns.begin()->second;
        columns.erase(columns.begin());
        for_each(columns.begin(), columns.end(), [&](const Pair<double, String> &col) {
            cols += ", " + col.second;
        });
        show_error("Column in group by clause must be in select clause: " + cols);
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
            show_error("Column index in order by must be greater than 0");
        }
    }
}

/**
 * check if the statement is valid
 * @param stmt the statement to be checked
 * @param after_where if we want to check clause after WHERE
 */
static void validate_stmt(SelectStatement *stmt, TableSet *from_set, bool after_where = false) {
    if (after_where) {
        // validate group_by and order_by
        validate_group_by(from_set, stmt->select, stmt->group);
        validate_order_by(stmt->order);
        return;
    }
    validate_select(from_set, stmt->select);
    validate_where(from_set, stmt->where);
}

/**
 * parse SELECT clause
 * @return vector of SELECT node
 */
static Vector<SelectNode> *parse_select() {
    val select = new Vector<SelectNode>();
    Token *tk;
    do {
        String as;
        if (peek()->type == T_STAR) {
            pop();
            val col = new ASTNode(A_COLUMN, D_NONE, null, null, "*");
            // select *
            select->push_back({col, as});
            continue;
        }

        val col = expression();
        if (col->atype == A_COLUMN) {
            val col_name = resolve_column(col->text)->second;
            if (col_name != "*") {
                as = col_name;
            } else {
                // select t.*
                select->push_back({col, as});
                continue;
            }
        }
        if ((tk = peek())->type == T_AS || tk->type == T_IDENTIFIER) {
            if (tk->type == T_AS) {
                pop();
            }
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
 * @return the FROM node
 */
static Pair<ASTNode *, TableSet *> parse_from() {
    Token *tk = null;
    ASTNode *join = null;
    val from = new TableSet();
    var index = 0;
    val alias = new Map<String, int>();

    do {
        val name = match(T_IDENTIFIER, "table name")->text;
        String as;

        if (!TABLES.count(name)) {
            show_error("Table " + name + " not found");
        }
        join = new ASTNode(A_JOIN, D_NONE, join, null, name);
        alias->insert({name, 1}); // 1 is meaningless

        if ((tk = peek())->type == T_AS || tk->type == T_IDENTIFIER) {
            if (tk->type == T_AS) {
                pop();
            }
            as = match(T_IDENTIFIER, "alias name")->text;
            if (alias->count(as)) {
                show_error("Duplicate alias name: " + as);
            }
            alias->insert({as, 1});
        }

        val table = TABLES.at(name);
        for (val &col: *table) {
            from->insert({name + "." + col.first, {col.second, index}});
            if (!as.empty()) {
                from->insert({as + "." + col.first, {col.second, index}});
            }
            if (from->count(col.first)) {
                from->at(col.first) = {col.second, -1};
            } else {
                from->insert({col.first, {col.second, index}});
            }
            index++;
        }
        TABLES_USED.insert({as.empty() ? name : as, name});

        if ((tk = peek())->type == T_ON) {
            pop();
            join->right = expression();
            validate_where(from, join->right);
        } else {
            join->right = new ASTNode(A_LITERAL, D_BOOL, null, null, true);
        }
    } while ((tk = pop())->type == T_JOIN); // maybe left/right/full join, in the future
    unpop();

    return {join, from};
}

/**
 * generate from node of "from dual", when no from clause, use the dummy table "dual"
 * @return the FROM node
 */
static Pair<ASTNode *, TableSet *> from_dual() {
    val from = new TableSet();
    from->insert({"<dual>.<dual>", {D_NONE, 0}});

    val true_node = new ASTNode(A_LITERAL, D_BOOL, null, null, true);
    val node = new ASTNode(A_JOIN, D_NONE, null, true_node, DUAL);
    return {node, from};
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
            order = token->integer - 1;
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
    int count = match(T_INTEGER, "integer")->integer, offset = 0;
    TokenType type = peek()->type;
    if (type == T_COMMA || type == T_OFFSET) {
        pop(); // comma/offset
        offset = match(T_INTEGER, "integer")->integer;
        if (type == T_COMMA) {
            int temp = offset;
            offset = count;
            count = temp;
        }
    }

    node->count = count;
    node->offset = offset;

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

    Pair<ASTNode *, TableSet *> from_pair;
    if (peek()->type == T_FROM) {
        pop();
        from_pair = parse_from();
    } else {
        from_pair = from_dual();
    }

    if (peek()->type == T_WHERE) {
        pop();
        stmt->where = expression();
    }

    validate_stmt(stmt, from_pair.second);

    if (peek()->type == T_GROUP) {
        pop();
        stmt->group = parse_group_by();
    }

    if (peek()->type == T_ORDER) {
        pop();
        stmt->order = parse_order_by(stmt->select);
    }

    validate_stmt(stmt, from_pair.second, true);

    if (peek()->type == T_LIMIT) {
        pop();
        stmt->limit = parse_limit();
    }

    if (!name.empty()) {
        // add to table map
        TABLES.insert({name, SELECTS});
    }

    stmt->from = from_pair.first;
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
Statement parse_read(Vector<Token *> *tokens) {
    init_parser(tokens);

    SelectStatement *stmt;

    var tk = pop();
    if (tk->type == T_SELECT) {
        stmt = parse_select_stmt("");
    } else if (tk->type == T_WITH) {
        stmt = parse_with();
    }

    match(T_SEMICOLON, "semicolon at the end of sql statement");

    return Statement{S_SELECT, stmt};
}
