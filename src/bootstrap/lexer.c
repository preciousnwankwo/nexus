#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void lexer_advance(Lexer *lex) {
    if (lex->pos < lex->source_len) {
        if (lex->source[lex->pos] == '\n') {
            lex->line++;
            lex->col = 1;
        } else {
            lex->col++;
        }
        lex->pos++;
    }
}

static char lexer_peek_char_at(Lexer *lex, size_t offset) {
    size_t p = lex->pos + offset;
    if (p < lex->source_len) return lex->source[p];
    return '\0';
}

static void lexer_skip_whitespace(Lexer *lex) {
    while (lex->pos < lex->source_len) {
        char c = lex->source[lex->pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lexer_advance(lex);
        } else if (c == '/' && lexer_peek_char_at(lex, 1) == '/') {
            while (lex->pos < lex->source_len && lex->source[lex->pos] != '\n') {
                lexer_advance(lex);
            }
        } else if (c == '/' && lexer_peek_char_at(lex, 1) == '*') {
            lexer_advance(lex);
            lexer_advance(lex);
            while (lex->pos < lex->source_len) {
                if (lex->source[lex->pos] == '*' && lexer_peek_char_at(lex, 1) == '/') {
                    lexer_advance(lex);
                    lexer_advance(lex);
                    break;
                }
                lexer_advance(lex);
            }
        } else {
            break;
        }
    }
}

static Token lexer_make_token(TokenKind kind, const char *value, size_t len, int line, int col) {
    Token t;
    t.kind = kind;
    t.value = value;
    t.value_len = len;
    t.loc.line = line;
    t.loc.col = col;
    t.loc.file = NULL;
    return t;
}

static Token lexer_make_error(const char *msg, int line, int col) {
    return lexer_make_token(TOKEN_ERROR, msg, strlen(msg), line, col);
}

static TokenKind check_keyword(const char *str, size_t len) {
    struct { const char *kw; size_t len; TokenKind kind; } keywords[] = {
        {"fn", 2, TOKEN_FN}, {"let", 3, TOKEN_LET}, {"mut", 3, TOKEN_MUT},
        {"if", 2, TOKEN_IF}, {"else", 4, TOKEN_ELSE}, {"match", 5, TOKEN_MATCH},
        {"return", 6, TOKEN_RETURN}, {"struct", 6, TOKEN_STRUCT}, {"enum", 4, TOKEN_ENUM},
        {"import", 6, TOKEN_IMPORT}, {"export", 6, TOKEN_EXPORT}, {"packet", 6, TOKEN_PACKET},
        {"parallel", 8, TOKEN_PARALLEL}, {"async", 5, TOKEN_ASYNC}, {"await", 5, TOKEN_AWAIT},
        {"agent", 5, TOKEN_AGENT}, {"unsafe", 6, TOKEN_UNSAFE}, {"arena", 5, TOKEN_ARENA},
        {"own", 3, TOKEN_OWN}, {"borrow", 6, TOKEN_BORROW}, {"ref", 3, TOKEN_REF},
        {"true", 4, TOKEN_TRUE}, {"false", 5, TOKEN_FALSE}, {"nil", 3, TOKEN_NIL},
        {"and", 3, TOKEN_AND}, {"or", 2, TOKEN_OR}, {"not", 3, TOKEN_NOT},
        {"xor", 3, TOKEN_XOR}, {"shl", 3, TOKEN_SHL}, {"shr", 3, TOKEN_SHR},
        {"mod", 3, TOKEN_MOD}, {"as", 2, TOKEN_AS}, {"type", 4, TOKEN_TYPE},
        {"self", 4, TOKEN_SELF}, {"super", 5, TOKEN_SUPER},
    };
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (len == keywords[i].len && memcmp(str, keywords[i].kw, len) == 0) {
            return keywords[i].kind;
        }
    }
    return TOKEN_IDENT;
}

static Token lexer_read_ident(Lexer *lex) {
    int line = lex->line, col = lex->col;
    size_t start = lex->pos;
    while (lex->pos < lex->source_len && (isalnum((unsigned char)lex->source[lex->pos]) || lex->source[lex->pos] == '_')) {
        lexer_advance(lex);
    }
    size_t len = lex->pos - start;
    const char *str = arena_strndup(lex->arena, lex->source + start, len);
    TokenKind kind = check_keyword(str, len);
    return lexer_make_token(kind, str, len, line, col);
}

static Token lexer_read_number(Lexer *lex) {
    int line = lex->line, col = lex->col;
    size_t start = lex->pos;
    int is_float = 0;

    if (lex->source[lex->pos] == '0' && lex->pos + 1 < lex->source_len) {
        char next = lex->source[lex->pos + 1];
        if (next == 'x' || next == 'X') {
            lexer_advance(lex); // skip 0
            lexer_advance(lex); // skip x
            while (lex->pos < lex->source_len && isxdigit((unsigned char)lex->source[lex->pos])) {
                lexer_advance(lex);
            }
            size_t len = lex->pos - start;
            const char *str = arena_strndup(lex->arena, lex->source + start, len);
            return lexer_make_token(TOKEN_INT_LIT, str, len, line, col);
        }
        if (next == 'b' || next == 'B') {
            lexer_advance(lex); // skip 0
            lexer_advance(lex); // skip b
            while (lex->pos < lex->source_len && (lex->source[lex->pos] == '0' || lex->source[lex->pos] == '1')) {
                lexer_advance(lex);
            }
            size_t len = lex->pos - start;
            const char *str = arena_strndup(lex->arena, lex->source + start, len);
            return lexer_make_token(TOKEN_INT_LIT, str, len, line, col);
        }
    }

    while (lex->pos < lex->source_len && isdigit((unsigned char)lex->source[lex->pos])) {
        lexer_advance(lex);
    }

    if (lex->pos < lex->source_len && lex->source[lex->pos] == '.' &&
        lexer_peek_char_at(lex, 1) != '.' && !isspace((unsigned char)lexer_peek_char_at(lex, 1))) {
        is_float = 1;
        lexer_advance(lex);
        while (lex->pos < lex->source_len && isdigit((unsigned char)lex->source[lex->pos])) {
            lexer_advance(lex);
        }
    }

    if (is_float && lex->pos < lex->source_len && (lex->source[lex->pos] == 'e' || lex->source[lex->pos] == 'E')) {
        lexer_advance(lex);
        if (lex->pos < lex->source_len && (lex->source[lex->pos] == '+' || lex->source[lex->pos] == '-')) {
            lexer_advance(lex);
        }
        while (lex->pos < lex->source_len && isdigit((unsigned char)lex->source[lex->pos])) {
            lexer_advance(lex);
        }
    }

    size_t len = lex->pos - start;
    const char *str = arena_strndup(lex->arena, lex->source + start, len);
    return lexer_make_token(is_float ? TOKEN_FLOAT_LIT : TOKEN_INT_LIT, str, len, line, col);
}

static Token lexer_read_string(Lexer *lex) {
    int line = lex->line, col = lex->col;
    lexer_advance(lex); // skip opening quote

    size_t cap = 64;
    size_t len = 0;
    char *buf = arena_alloc(lex->arena, cap);

    while (lex->pos < lex->source_len && lex->source[lex->pos] != '"') {
        if (lex->source[lex->pos] == '\\') {
            lexer_advance(lex);
            if (lex->pos >= lex->source_len) return lexer_make_error("unterminated escape", line, col);
            char c;
            switch (lex->source[lex->pos]) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '\\': c = '\\'; break;
                case '"': c = '"'; break;
                case '0': c = '\0'; break;
                default: c = lex->source[lex->pos]; break;
            }
            if (len + 1 >= cap) {
                cap *= 2;
                char *new_buf = arena_alloc(lex->arena, cap);
                memcpy(new_buf, buf, len);
                buf = new_buf;
            }
            buf[len++] = c;
        } else {
            if (len + 1 >= cap) {
                cap *= 2;
                char *new_buf = arena_alloc(lex->arena, cap);
                memcpy(new_buf, buf, len);
                buf = new_buf;
            }
            buf[len++] = lex->source[lex->pos];
        }
        lexer_advance(lex);
    }

    if (lex->pos >= lex->source_len) return lexer_make_error("unterminated string", line, col);
    lexer_advance(lex); // skip closing quote

    buf[len] = '\0';
    return lexer_make_token(TOKEN_STRING_LIT, buf, len, line, col);
}

static Token lexer_read_char(Lexer *lex) {
    int line = lex->line, col = lex->col;
    lexer_advance(lex); // skip opening quote

    if (lex->pos >= lex->source_len) return lexer_make_error("unterminated char", line, col);

    char c;
    if (lex->source[lex->pos] == '\\') {
        lexer_advance(lex);
        if (lex->pos >= lex->source_len) return lexer_make_error("unterminated escape", line, col);
        switch (lex->source[lex->pos]) {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            case '\\': c = '\\'; break;
            case '\'': c = '\''; break;
            case '0': c = '\0'; break;
            default: c = lex->source[lex->pos]; break;
        }
    } else {
        c = lex->source[lex->pos];
    }
    lexer_advance(lex);

    if (lex->pos >= lex->source_len || lex->source[lex->pos] != '\'') {
        return lexer_make_error("unterminated char literal", line, col);
    }
    lexer_advance(lex); // skip closing quote

    char *buf = arena_alloc(lex->arena, 2);
    buf[0] = c;
    buf[1] = '\0';
    return lexer_make_token(TOKEN_CHAR_LIT, buf, 1, line, col);
}

static Token lexer_next_token(Lexer *lex) {
    lexer_skip_whitespace(lex);

    if (lex->pos >= lex->source_len) {
        return lexer_make_token(TOKEN_EOF, "", 0, lex->line, lex->col);
    }

    char c = lex->source[lex->pos];
    int line = lex->line, col = lex->col;

    if (isalpha((unsigned char)c) || c == '_') return lexer_read_ident(lex);
    if (isdigit((unsigned char)c)) return lexer_read_number(lex);
    if (c == '"') return lexer_read_string(lex);
    if (c == '\'') return lexer_read_char(lex);

    lexer_advance(lex);

    switch (c) {
        case '+': return lexer_make_token(TOKEN_PLUS, "+", 1, line, col);
        case '*': return lexer_make_token(TOKEN_STAR, "*", 1, line, col);
        case '/': return lexer_make_token(TOKEN_SLASH, "/", 1, line, col);
        case '%': return lexer_make_token(TOKEN_PERCENT, "%", 1, line, col);
        case '(': return lexer_make_token(TOKEN_LPAREN, "(", 1, line, col);
        case ')': return lexer_make_token(TOKEN_RPAREN, ")", 1, line, col);
        case '{': return lexer_make_token(TOKEN_LBRACE, "{", 1, line, col);
        case '}': return lexer_make_token(TOKEN_RBRACE, "}", 1, line, col);
        case '[': return lexer_make_token(TOKEN_LBRACKET, "[", 1, line, col);
        case ']': return lexer_make_token(TOKEN_RBRACKET, "]", 1, line, col);
        case ',': return lexer_make_token(TOKEN_COMMA, ",", 1, line, col);
        case ';': return lexer_make_token(TOKEN_SEMICOLON, ";", 1, line, col);
        case '@': return lexer_make_token(TOKEN_AT, "@", 1, line, col);
        case '#': return lexer_make_token(TOKEN_HASH, "#", 1, line, col);
        case '$': return lexer_make_token(TOKEN_DOLLAR, "$", 1, line, col);
        case '?': return lexer_make_token(TOKEN_QUESTION, "?", 1, line, col);
        case '~': return lexer_make_token(TOKEN_TILDE, "~", 1, line, col);
        case '.':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '.') {
                lexer_advance(lex);
                if (lex->pos < lex->source_len && lex->source[lex->pos] == '.') {
                    lexer_advance(lex);
                    return lexer_make_token(TOKEN_DOT_DOT_DOT, "...", 3, line, col);
                }
                return lexer_make_token(TOKEN_DOT_DOT, "..", 2, line, col);
            }
            return lexer_make_token(TOKEN_DOT, ".", 1, line, col);
        case ':':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == ':') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_COLON_COLON, "::", 2, line, col);
            }
            return lexer_make_token(TOKEN_COLON, ":", 1, line, col);
        case '=':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '=') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_EQEQ, "==", 2, line, col);
            }
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '>') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_FAT_ARROW, "=>", 2, line, col);
            }
            return lexer_make_token(TOKEN_EQ, "=", 1, line, col);
        case '!':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '=') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_NEQ, "!=", 2, line, col);
            }
            return lexer_make_token(TOKEN_BANG, "!", 1, line, col);
        case '<':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '=') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_LE, "<=", 2, line, col);
            }
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '<') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_SHL, "<<", 2, line, col);
            }
            return lexer_make_token(TOKEN_LT, "<", 1, line, col);
        case '>':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '=') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_GE, ">=", 2, line, col);
            }
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '>') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_SHR, ">>", 2, line, col);
            }
            return lexer_make_token(TOKEN_GT, ">", 1, line, col);
        case '-':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '>') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_ARROW, "->", 2, line, col);
            }
            return lexer_make_token(TOKEN_MINUS, "-", 1, line, col);
        case '|':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '|') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_OR, "||", 2, line, col);
            }
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '=') {
                lexer_advance(lex);
                if (lex->pos < lex->source_len && lex->source[lex->pos] == '>') {
                    lexer_advance(lex);
                    return lexer_make_token(TOKEN_PIPE_ARROW, "|=>", 3, line, col);
                }
                return lexer_make_token(TOKEN_ERROR, "|=", 2, line, col);
            }
            return lexer_make_token(TOKEN_PIPE, "|", 1, line, col);
        case '&':
            if (lex->pos < lex->source_len && lex->source[lex->pos] == '&') {
                lexer_advance(lex);
                return lexer_make_token(TOKEN_AND, "&&", 2, line, col);
            }
            return lexer_make_token(TOKEN_AMP, "&", 1, line, col);
        case '^':
            return lexer_make_token(TOKEN_CARET, "^", 1, line, col);
        case '_':
            return lexer_make_token(TOKEN_WILDCARD, "_", 1, line, col);
        default:
            return lexer_make_token(TOKEN_ERROR, &lex->source[lex->pos - 1], 1, line, col);
    }
}

void lexer_init(Lexer *lex, const char *source, size_t source_len, Arena *arena, StringTable *strings) {
    lex->source = source;
    lex->source_len = source_len;
    lex->pos = 0;
    lex->line = 1;
    lex->col = 1;
    lex->arena = arena;
    lex->strings = strings;
    lex->has_peek = 0;
    lex->current = lexer_next_token(lex);
    lex->peek = lexer_make_token(TOKEN_EOF, "", 0, 0, 0);
}

Token lexer_next(Lexer *lex) {
    if (lex->has_peek) {
        lex->has_peek = 0;
        lex->current = lex->peek;
        return lex->current;
    }
    Token result = lex->current;
    lex->current = lexer_next_token(lex);
    return result;
}

Token lexer_peek(Lexer *lex) {
    if (!lex->has_peek) {
        lex->peek = lexer_next_token(lex);
        lex->has_peek = 1;
    }
    return lex->peek;
}

Token lexer_expect(Lexer *lex, TokenKind kind) {
    Token t = lexer_next(lex);
    if (t.kind != kind) {
        fprintf(stderr, "%s:%d:%d: expected %s, got %s\n",
                t.loc.file ? t.loc.file : "<input>",
                t.loc.line, t.loc.col,
                token_kind_name(kind), token_kind_name(t.kind));
        exit(1);
    }
    return t;
}

int lexer_match(Lexer *lex, TokenKind kind) {
    if (lexer_peek(lex).kind == kind) {
        lexer_next(lex);
        return 1;
    }
    return 0;
}

const char *token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENT: return "identifier";
        case TOKEN_INT_LIT: return "integer literal";
        case TOKEN_FLOAT_LIT: return "float literal";
        case TOKEN_STRING_LIT: return "string literal";
        case TOKEN_CHAR_LIT: return "char literal";
        case TOKEN_FN: return "fn";
        case TOKEN_LET: return "let";
        case TOKEN_MUT: return "mut";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_MATCH: return "match";
        case TOKEN_RETURN: return "return";
        case TOKEN_STRUCT: return "struct";
        case TOKEN_ENUM: return "enum";
        case TOKEN_IMPORT: return "import";
        case TOKEN_EXPORT: return "export";
        case TOKEN_PACKET: return "packet";
        case TOKEN_PARALLEL: return "parallel";
        case TOKEN_ASYNC: return "async";
        case TOKEN_AWAIT: return "await";
        case TOKEN_AGENT: return "agent";
        case TOKEN_UNSAFE: return "unsafe";
        case TOKEN_ARENA: return "arena";
        case TOKEN_OWN: return "own";
        case TOKEN_BORROW: return "borrow";
        case TOKEN_REF: return "ref";
        case TOKEN_TRUE: return "true";
        case TOKEN_FALSE: return "false";
        case TOKEN_NIL: return "nil";
        case TOKEN_AND: return "and";
        case TOKEN_OR: return "or";
        case TOKEN_NOT: return "not";
        case TOKEN_XOR: return "xor";
        case TOKEN_SHL: return "shl";
        case TOKEN_SHR: return "shr";
        case TOKEN_MOD: return "mod";
        case TOKEN_AS: return "as";
        case TOKEN_TYPE: return "type";
        case TOKEN_SELF: return "self";
        case TOKEN_SUPER: return "super";
        case TOKEN_WILDCARD: return "_";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_PERCENT: return "%";
        case TOKEN_EQ: return "=";
        case TOKEN_EQEQ: return "==";
        case TOKEN_NEQ: return "!=";
        case TOKEN_LT: return "<";
        case TOKEN_GT: return ">";
        case TOKEN_LE: return "<=";
        case TOKEN_GE: return ">=";
        case TOKEN_AMP: return "&";
        case TOKEN_PIPE: return "|";
        case TOKEN_CARET: return "^";
        case TOKEN_TILDE: return "~";
        case TOKEN_BANG: return "!";
        case TOKEN_ARROW: return "->";
        case TOKEN_FAT_ARROW: return "=>";
        case TOKEN_PIPE_ARROW: return "|=>";
        case TOKEN_COLON: return ":";
        case TOKEN_COLON_COLON: return "::";
        case TOKEN_DOT: return ".";
        case TOKEN_DOT_DOT: return "..";
        case TOKEN_DOT_DOT_DOT: return "...";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_COMMA: return ",";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_AT: return "@";
        case TOKEN_HASH: return "#";
        case TOKEN_DOLLAR: return "$";
        case TOKEN_QUESTION: return "?";
        case TOKEN_NEWLINE: return "\\n";
        case TOKEN_ERROR: return "error";
    }
    return "unknown";
}
