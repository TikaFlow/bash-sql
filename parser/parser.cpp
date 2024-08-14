//
// Created by tika on 24-8-13.
//

#include "parser.h"

static int INDEX = 0;
static const String *SQL;
static Map<String, TokenType> *MAP;

static void init_keyword_map() {
    MAP = new Map<String, TokenType>();
    using Pair = std::pair<String, TokenType>;
    MAP->insert(Pair("select", T_SELECT));
    MAP->insert(Pair("as", T_AS));
    MAP->insert(Pair("from", T_FROM));
    MAP->insert(Pair("where", T_WHERE));
    MAP->insert(Pair("join", T_JOIN));
    MAP->insert(Pair("on", T_ON));
    MAP->insert(Pair("group", T_GROUP));
    MAP->insert(Pair("by", T_BY));
    MAP->insert(Pair("order", T_ORDER));
    MAP->insert(Pair("having", T_HAVING));
    MAP->insert(Pair("in", T_IN));
    MAP->insert(Pair("offset", T_OFFSET));
    MAP->insert(Pair("limit", T_LIMIT));
    MAP->insert(Pair("and", T_AND));
    MAP->insert(Pair("or", T_OR));
    MAP->insert(Pair("not", T_NOT));
    MAP->insert(Pair("is", T_IS));
    MAP->insert(Pair("null", T_NULL));
    MAP->insert(Pair("when", T_WHEN));
    MAP->insert(Pair("then", T_THEN));
    MAP->insert(Pair("else", T_ELSE));
    MAP->insert(Pair("with", T_WITH));
}

static char next() {
    static val len = SQL->length();
    if (INDEX < len) {
        return SQL->at(INDEX++);
    }
    return EOF;
}

static void prev() {
    INDEX--;
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
                prev();
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
    val start = INDEX;
    while ((c = next()) != EOF && isdigit(c));

    if (c == '.') {
        if ((c = next()) == EOF || !isdigit(c)) {
            show_error("invalid number");
        } else {
            while ((c = next()) != EOF && isdigit(c));
        }
    }

    token->type = T_NUMBER;
    String str = SQL->substr(start, INDEX - start);
    token->number = stod(str);
}

static void check_keyword(Token *token) {
    String text;

    std::transform(token->text->begin(), token->text->end(), std::back_inserter(text), ::tolower);
    if (MAP->count(text) > 0) {
        token->type = MAP->at(text);
    }
}

static void scan_identifier(Token *token) {
    char c;
    val start = INDEX;
    while ((c = next()) != EOF && (c == '_' || isalpha(c) || isdigit(c)));

    token->type = T_IDENTIFIER;
    String str = SQL->substr(start, INDEX - start);
    token->text = new String(str);

    check_keyword(token);
}

static Token *scan() {
    Token *token = new_token();
    char c = skip();

    switch (c) {
        case EOF:
            token = null;
            break;
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

    return token;
}

static Vector<Token *> *lex(const String *sql) {
    SQL = sql;
    Token *tk;
    val tokens = new Vector<Token *>();
    init_keyword_map();

    while ((tk = scan())) {
        tokens->push_back(tk);
    }

    return tokens;
}

static ASTNode *parse(Vector<Token *> *tokens) {
    // TODO implement
    return null;
}

ASTNode *parse(const String *sql) {
    if (!sql || sql->empty()) {
        return null;
    }
    val tokens = lex(sql);
    return parse(tokens);
}
