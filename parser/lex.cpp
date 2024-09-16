//
// Created by tika on 24-8-14.
//

#include "lex.h"

static int INDEX = 0;
static String SQL;
static Map<String, TokenType> KEYWORD;
static Map<TokenType, String> DESC;

/**
 * initialize the keyword map and description map
 */
static void init_map() {
    KEYWORD.insert({"select", T_SELECT});
    KEYWORD.insert({"as", T_AS});
    KEYWORD.insert({"from", T_FROM});
    KEYWORD.insert({"join", T_JOIN});
    KEYWORD.insert({"on", T_ON});
    KEYWORD.insert({"where", T_WHERE});
    KEYWORD.insert({"true", T_TRUE});
    KEYWORD.insert({"false", T_FALSE});
    KEYWORD.insert({"group", T_GROUP});
    KEYWORD.insert({"by", T_BY});
    KEYWORD.insert({"order", T_ORDER});
    KEYWORD.insert({"asc", T_ASC});
    KEYWORD.insert({"desc", T_DESC});
    KEYWORD.insert({"in", T_IN});
    KEYWORD.insert({"offset", T_OFFSET});
    KEYWORD.insert({"limit", T_LIMIT});
    KEYWORD.insert({"and", T_AND});
    KEYWORD.insert({"or", T_OR});
    KEYWORD.insert({"not", T_NOT});
    KEYWORD.insert({"like", T_LIKE});
    KEYWORD.insert({"is", T_IS});
    KEYWORD.insert({"null", T_NULL});
    KEYWORD.insert({"with", T_WITH});

    DESC.insert({T_EOF, "EOF"});
    DESC.insert({T_PLUS, "+"});
    DESC.insert({T_MINUS, "-"});
    DESC.insert({T_STAR, "*"});
    DESC.insert({T_SLASH, "/"});
    DESC.insert({T_EQ, "="});
    DESC.insert({T_NE1, "<>"});
    DESC.insert({T_NE2, "!="});
    DESC.insert({T_LT, "<"});
    DESC.insert({T_GT, ">"});
    DESC.insert({T_LE, "<="});
    DESC.insert({T_GE, ">="});
    DESC.insert({T_LPAREN, "("});
    DESC.insert({T_RPAREN, ")"});
    DESC.insert({T_COMMA, ","});
    DESC.insert({T_SEMICOLON, ";"});
    DESC.insert({T_INTEGER, "integer"});
    DESC.insert({T_REAL, "real number"});
}

/**
 * get the next character from the SQL string
 * @return the next character
 */
static char next() {
    static val len = SQL.length();
    if (INDEX >= len) {
        return EOF;
    }
    return SQL.at(INDEX++);
}

/**
 * push back the character to the SQL string, so that it can be read again
 * if the character is EOF, do nothing
 * @param c the character to be pushed back
 */
static void prev(char c) {
    if (c == EOF) {
        return;
    }
    if (INDEX > 0) {
        INDEX--;
    }
}

/**
 * skip the line comment
 * @return the first character after the line comment
 */
static char skip_line_comment() {
    char c;
    while ((c = next()) != '\n' && c != EOF);

    return c;
}

/**
 * skip the white space, line comment
 * @return the first character we can use
 */
static char skip() {
    char c = next();

    while (true) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
            c = next();
        } else if (c == '-') {
            char n = next();
            if (n == '-') {
                c = skip_line_comment();
            } else {
                prev(n);
                break;
            }
        } else {
            break;
        }
    }

    return c;
}

/**
 * escape character in string
 * @param str the string to be escaped
 * @return the escaped string
 */
static String escape_string(String &str) {
    std::ostringstream oss;
    val len = str.length();
    size_t pos = 0;

    while (pos < len) {
        char c = str.at(pos);
        if (c == '\\') {
            pos++;
            c = str.at(pos);
            switch (c) {
                case 'n':
                    oss << '\n';
                    break;
                case 'r':
                    oss << '\r';
                    break;
                case 't':
                    oss << '\t';
                    break;
                case 'f':
                    oss << '\f';
                    break;
                case '0':
                    oss << '\0';
                    break;
                case '\'':
                    oss << '\'';
                    break;
                case '\"':
                    oss << '\"';
                    break;
                case '\\':
                    oss << '\\';
                    break;
                default:
                    oss << c;
                    break;
            }
        } else {
            oss << c;
        }
        pos++;
    }

    return oss.str();
}

/**
 * check if the token is a keyword
 * @param token the token to be checked
 */
static void check_keyword(Token *token) {
    val text = to_lower(token->text);
    if (KEYWORD.count(text)) {
        token->type = KEYWORD.at(text);
        token->text = text; // prevent mixed case
    }
}

/**
 * describe the token
 * @param token the token to be described
 */
static void describe(Token *token) {
    if (DESC.count(token->type)) {
        token->text = DESC.at(token->type);
    }
}

/**
 * scan a string starting with quote(' or "), and save it to token
 * @param token the token to be scanned
 * @param quote the quote character
 */
static void scan_string(Token *token, char quote) {
    char c;
    val start = INDEX;
    while ((c = next()) != EOF && c != quote) {
        if (c == '\\') {
            next();
        }
    }
    if (c == EOF) {
        show_error("unterminated string");
    } else {
        token->type = T_STRING;
        String str = SQL.substr(start, INDEX - start - 1);
        token->text = escape_string(str);
    }
}

/**
 * scan a number, and save it to token
 * number can be integer or real
 * @param token the token to be scanned
 */
static void scan_number(Token *token) {
    char c;
    var dot = false;
    val start = INDEX - 1;
    while ((c = next()) != EOF && isdigit(c));

    if (c == '.') {
        if ((c = next()) == EOF || !isdigit(c)) {
            show_error("invalid number");
        } else {
            while ((c = next()) != EOF && isdigit(c));
        }
        dot = true;
    }

    token->type = dot ? T_REAL : T_INTEGER;
    prev(c);
    String str = SQL.substr(start, INDEX - start);
    if (dot) {
        token->real = stod(str);
    } else {
        token->integer = stoi(str);
    }
}

/**
 * scan an identifier, and save it to token, after that, check if it is a keyword
 * @param token the token to be scanned
 */
static void scan_identifier(Token *token) {
    char c;
    val start = INDEX - 1;

    while ((c = next()) != EOF && (c == '_' || isalpha(c) || isdigit(c)));
    if (c == '.') {
        c = next();
        if (c != '*') {
            prev(c);
            while ((c = next()) != EOF && (c == '_' || isalpha(c) || isdigit(c)));
            prev(c);
        }
    } else {
        prev(c);
    }

    token->type = T_IDENTIFIER;
    token->text = SQL.substr(start, INDEX - start);

    check_keyword(token);
}

/**
 * scan a token
 * @return the token scanned
 */
static Token *scan() {
    val token = new Token();
    char c = skip();

    switch (c) {
        case EOF:
            return null;
        case '+':
            token->type = T_PLUS;
            break;
        case '-':
            token->type = T_MINUS;
            break;
        case '*':
            token->type = T_STAR;
            break;
        case '/':
            token->type = T_SLASH;
            break;
        case '=':
            token->type = T_EQ;
            break;
        case '<':
            if ((c = next()) == '=') {
                token->type = T_LE;
            } else if (c == '>') {
                token->type = T_NE1;
            } else {
                token->type = T_LT;
                prev(c);
            }
            break;
        case '>':
            if (next() == '=') {
                token->type = T_GE;
            } else {
                token->type = T_GT;
                prev(c);
            }
            break;
        case '!':
            if (next() == '=') {
                token->type = T_NE2;
            } else {
                show_error("Invalid character: " + String(1, c));
            }
            break;
        case '(':
            token->type = T_LPAREN;
            break;
        case ')':
            token->type = T_RPAREN;
            break;
        case ',':
            token->type = T_COMMA;
            break;
        case ';':
            token->type = T_SEMICOLON;
            break;
        case '\'':
        case '\"':
            scan_string(token, c);
            break;
        default:
            if (isdigit(c)) {
                scan_number(token);
                break;
            }
            if (isalpha(c)) {
                scan_identifier(token);
                break;
            }
            show_error("Invalid character: " + String(1, c));
    }

    describe(token);
    return token;
}

/**
 * scan tokens from a sql string
 * @param sql the sql string to be scanned
 * @return vector, which contains all tokens
 */
Vector<Token *> *lex(const String &sql) {
    SQL = sql;
    init_map();

    val tokens = new Vector<Token *>();
    Token *tk;

    while ((tk = scan())) {
        tokens->push_back(tk);
    }

    return tokens;
}
