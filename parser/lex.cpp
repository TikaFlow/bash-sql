//
// Created by tika on 24-8-14.
//

#include "lex.h"

static int INDEX = 0;
static const String *SQL;
static Map<String, TokenType> *KEYWORD;
static Map<TokenType, String> *DESC;

static void init_map() {
    KEYWORD = new Map<String, TokenType>();
    using Keyword = std::pair<String, TokenType>;
    KEYWORD->insert(Keyword("select", T_SELECT));
    KEYWORD->insert(Keyword("as", T_AS));
    KEYWORD->insert(Keyword("from", T_FROM));
    KEYWORD->insert(Keyword("where", T_WHERE));
    KEYWORD->insert(Keyword("join", T_JOIN));
    KEYWORD->insert(Keyword("on", T_ON));
    KEYWORD->insert(Keyword("group", T_GROUP));
    KEYWORD->insert(Keyword("by", T_BY));
    KEYWORD->insert(Keyword("order", T_ORDER));
    KEYWORD->insert(Keyword("having", T_HAVING));
    KEYWORD->insert(Keyword("in", T_IN));
    KEYWORD->insert(Keyword("offset", T_OFFSET));
    KEYWORD->insert(Keyword("limit", T_LIMIT));
    KEYWORD->insert(Keyword("and", T_AND));
    KEYWORD->insert(Keyword("or", T_OR));
    KEYWORD->insert(Keyword("not", T_NOT));
    KEYWORD->insert(Keyword("is", T_IS));
    KEYWORD->insert(Keyword("null", T_NULL));
    KEYWORD->insert(Keyword("when", T_WHEN));
    KEYWORD->insert(Keyword("then", T_THEN));
    KEYWORD->insert(Keyword("else", T_ELSE));
    KEYWORD->insert(Keyword("with", T_WITH));

    DESC = new Map<TokenType, String>();
    using Description = std::pair<TokenType, String>;
    DESC->insert(Description(T_EOF, "EOF"));
    DESC->insert(Description(T_PLUS, "+"));
    DESC->insert(Description(T_MINUS, "-"));
    DESC->insert(Description(T_STAR, "*"));
    DESC->insert(Description(T_SLASH, "/"));
    DESC->insert(Description(T_MOD, "%"));
    DESC->insert(Description(T_EQ, "="));
    DESC->insert(Description(T_NE1, "<>"));
    DESC->insert(Description(T_NE2, "!="));
    DESC->insert(Description(T_LT, "<"));
    DESC->insert(Description(T_GT, ">"));
    DESC->insert(Description(T_LE, "<="));
    DESC->insert(Description(T_GE, ">="));
    DESC->insert(Description(T_LPAREN, "("));
    DESC->insert(Description(T_RPAREN, ")"));
    DESC->insert(Description(T_COMMA, ","));
    DESC->insert(Description(T_SEMICOLON, ";"));
    DESC->insert(Description(T_NUMBER, "number"));
}

static char next() {
    static val len = SQL->length();
    if (INDEX < len) {
        return SQL->at(INDEX++);
    }
    return EOF;
}

static void prev(char c) {
    if (c == EOF) {
        return;
    }
    if (INDEX > 0) {
        INDEX--;
    }
}

static char skip_line_comment() {
    char c;
    while ((c = next()) != '\n' && c != EOF);

    return c;
}

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

static Token *new_token() {
    val token = new Token();
    token->type = T_EOF;
    token->number = 0;
    token->text = null;
    return token;
}

static String *escape_string(String &str) {
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

    return new String(oss.str());
}

static void check_keyword(Token *token) {
    String text;

    std::transform(token->text->begin(), token->text->end(), std::back_inserter(text), ::tolower);
    if (KEYWORD->count(text) > 0) {
        token->type = KEYWORD->at(text);
    }
}

static void describe(Token *token) {
    if (DESC->count(token->type) > 0) {
        token->text = new String(DESC->at(token->type));
    }
}

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
        String str = SQL->substr(start, INDEX - start - 1);
        token->text = escape_string(str);
    }
}

static void scan_number(Token *token) {
    char c;
    val start = INDEX - 1;
    while ((c = next()) != EOF && isdigit(c));

    if (c == '.') {
        if ((c = next()) == EOF || !isdigit(c)) {
            show_error("invalid number");
        } else {
            while ((c = next()) != EOF && isdigit(c));
        }
    }

    token->type = T_NUMBER;
    prev(c);
    String str = SQL->substr(start, INDEX - start);
    token->number = stod(str);
}

static void scan_identifier(Token *token) {
    char c;
    val start = INDEX - 1;
    while ((c = next()) != EOF && (c == '_' || isalpha(c) || isdigit(c)));

    token->type = T_IDENTIFIER;
    prev(c);
    String str = SQL->substr(start, INDEX - start);
    token->text = new String(str);

    check_keyword(token);
}

static Token *scan() {
    Token *token = new_token();
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
        case '%':
            token->type = T_MOD;
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
            if (c == '_' || isalpha(c)) {
                scan_identifier(token);
                break;
            }
            show_error("Invalid character: " + String(1, c));
    }

    describe(token);
    return token;
}

Vector<Token *> *lex(const String *sql) {
    SQL = sql;
    Token *tk;
    val tokens = new Vector<Token *>();
    init_map();

    while ((tk = scan())) {
        tokens->push_back(tk);
    }

    return tokens;
}
