#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/bootstrap/arena.h"
#include "../../src/bootstrap/string_table.h"
#include "../../src/bootstrap/lexer.h"

static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

static void assert_token(Lexer *lex, TokenKind expected, const char *desc) {
    Token t = lexer_next(lex);
    test_count++;
    if (t.kind == expected) {
        pass_count++;
    } else {
        fail_count++;
        fprintf(stderr, "FAIL: %s at %d:%d — expected %s, got %s (value: '%.*s')\n",
                desc, t.loc.line, t.loc.col,
                token_kind_name(expected), token_kind_name(t.kind),
                (int)t.value_len, t.value);
    }
}

static void assert_token_value(Lexer *lex, TokenKind expected, const char *value, const char *desc) {
    Token t = lexer_next(lex);
    test_count++;
    if (t.kind == expected && t.value_len == strlen(value) && memcmp(t.value, value, t.value_len) == 0) {
        pass_count++;
    } else {
        fail_count++;
        fprintf(stderr, "FAIL: %s at %d:%d — expected %s '%s', got %s '%.*s'\n",
                desc, t.loc.line, t.loc.col,
                token_kind_name(expected), value,
                token_kind_name(t.kind), (int)t.value_len, t.value);
    }
}

static Lexer make_lexer(const char *source) {
    static Arena arena;
    static StringTable strings;
    arena_init(&arena);
    string_table_init(&strings, &arena);
    Lexer lex;
    lexer_init(&lex, source, strlen(source), &arena, &strings);
    return lex;
}

static void test_keywords(void) {
    printf("  keywords...\n");
    Lexer lex = make_lexer("fn let mut if else match return struct enum import export packet parallel async agent unsafe arena own borrow ref");
    assert_token(&lex, TOKEN_FN, "fn");
    assert_token(&lex, TOKEN_LET, "let");
    assert_token(&lex, TOKEN_MUT, "mut");
    assert_token(&lex, TOKEN_IF, "if");
    assert_token(&lex, TOKEN_ELSE, "else");
    assert_token(&lex, TOKEN_MATCH, "match");
    assert_token(&lex, TOKEN_RETURN, "return");
    assert_token(&lex, TOKEN_STRUCT, "struct");
    assert_token(&lex, TOKEN_ENUM, "enum");
    assert_token(&lex, TOKEN_IMPORT, "import");
    assert_token(&lex, TOKEN_EXPORT, "export");
    assert_token(&lex, TOKEN_PACKET, "packet");
    assert_token(&lex, TOKEN_PARALLEL, "parallel");
    assert_token(&lex, TOKEN_ASYNC, "async");
    assert_token(&lex, TOKEN_AGENT, "agent");
    assert_token(&lex, TOKEN_UNSAFE, "unsafe");
    assert_token(&lex, TOKEN_ARENA, "arena");
    assert_token(&lex, TOKEN_OWN, "own");
    assert_token(&lex, TOKEN_BORROW, "borrow");
    assert_token(&lex, TOKEN_REF, "ref");
}

static void test_operators(void) {
    printf("  operators...\n");
    Lexer lex = make_lexer("+ - * / % = == != < > <= >= && || ! & | ^ ~ -> => | => :: . .. ...");
    assert_token(&lex, TOKEN_PLUS, "+");
    assert_token(&lex, TOKEN_MINUS, "-");
    assert_token(&lex, TOKEN_STAR, "*");
    assert_token(&lex, TOKEN_SLASH, "/");
    assert_token(&lex, TOKEN_PERCENT, "%");
    assert_token(&lex, TOKEN_EQ, "=");
    assert_token(&lex, TOKEN_EQEQ, "==");
    assert_token(&lex, TOKEN_NEQ, "!=");
    assert_token(&lex, TOKEN_LT, "<");
    assert_token(&lex, TOKEN_GT, ">");
    assert_token(&lex, TOKEN_LE, "<=");
    assert_token(&lex, TOKEN_GE, ">=");
    assert_token(&lex, TOKEN_AND, "&&");
    assert_token(&lex, TOKEN_OR, "||");
    assert_token(&lex, TOKEN_BANG, "!");
    assert_token(&lex, TOKEN_AMP, "&");
    assert_token(&lex, TOKEN_PIPE, "|");
    assert_token(&lex, TOKEN_CARET, "^");
    assert_token(&lex, TOKEN_TILDE, "~");
    assert_token(&lex, TOKEN_ARROW, "->");
    assert_token(&lex, TOKEN_FAT_ARROW, "=>");
    assert_token(&lex, TOKEN_PIPE, "|");
    assert_token(&lex, TOKEN_FAT_ARROW, "=>");
    assert_token(&lex, TOKEN_COLON_COLON, "::");
    assert_token(&lex, TOKEN_DOT, ".");
    assert_token(&lex, TOKEN_DOT_DOT, "..");
    assert_token(&lex, TOKEN_DOT_DOT_DOT, "...");
}

static void test_delimiters(void) {
    printf("  delimiters...\n");
    Lexer lex = make_lexer("( ) { } [ ] , ; @ # $ ?");
    assert_token(&lex, TOKEN_LPAREN, "(");
    assert_token(&lex, TOKEN_RPAREN, ")");
    assert_token(&lex, TOKEN_LBRACE, "{");
    assert_token(&lex, TOKEN_RBRACE, "}");
    assert_token(&lex, TOKEN_LBRACKET, "[");
    assert_token(&lex, TOKEN_RBRACKET, "]");
    assert_token(&lex, TOKEN_COMMA, ",");
    assert_token(&lex, TOKEN_SEMICOLON, ";");
    assert_token(&lex, TOKEN_AT, "@");
    assert_token(&lex, TOKEN_HASH, "#");
    assert_token(&lex, TOKEN_DOLLAR, "$");
    assert_token(&lex, TOKEN_QUESTION, "?");
}

static void test_integer_literals(void) {
    printf("  integer literals...\n");
    Lexer lex = make_lexer("0 42 123456789 0xff 0b1010");
    assert_token_value(&lex, TOKEN_INT_LIT, "0", "0");
    assert_token_value(&lex, TOKEN_INT_LIT, "42", "42");
    assert_token_value(&lex, TOKEN_INT_LIT, "123456789", "123456789");
    assert_token_value(&lex, TOKEN_INT_LIT, "0xff", "0xff");
    assert_token_value(&lex, TOKEN_INT_LIT, "0b1010", "0b1010");
}

static void test_float_literals(void) {
    printf("  float literals...\n");
    Lexer lex = make_lexer("3.14 0.5 1.0e10 2.5e-3");
    assert_token_value(&lex, TOKEN_FLOAT_LIT, "3.14", "3.14");
    assert_token_value(&lex, TOKEN_FLOAT_LIT, "0.5", "0.5");
    assert_token_value(&lex, TOKEN_FLOAT_LIT, "1.0e10", "1.0e10");
    assert_token_value(&lex, TOKEN_FLOAT_LIT, "2.5e-3", "2.5e-3");
}

static void test_string_literals(void) {
    printf("  string literals...\n");
    Lexer lex = make_lexer("\"hello world\" \"escape\\nnewline\" \"\"");
    assert_token_value(&lex, TOKEN_STRING_LIT, "hello world", "hello world");
    assert_token_value(&lex, TOKEN_STRING_LIT, "escape\nnewline", "escape newline");
    assert_token_value(&lex, TOKEN_STRING_LIT, "", "empty string");
}

static void test_char_literals(void) {
    printf("  char literals...\n");
    Lexer lex = make_lexer("'a' '\\n' '\\0'");
    assert_token(&lex, TOKEN_CHAR_LIT, "'a'");
    assert_token(&lex, TOKEN_CHAR_LIT, "'\\n'");
    assert_token(&lex, TOKEN_CHAR_LIT, "'\\0'");
}

static void test_identifiers(void) {
    printf("  identifiers...\n");
    Lexer lex = make_lexer("foo bar_baz _private x123");
    assert_token_value(&lex, TOKEN_IDENT, "foo", "foo");
    assert_token_value(&lex, TOKEN_IDENT, "bar_baz", "bar_baz");
    assert_token_value(&lex, TOKEN_IDENT, "_private", "_private");
    assert_token_value(&lex, TOKEN_IDENT, "x123", "x123");
}

static void test_comments(void) {
    printf("  comments...\n");
    Lexer lex = make_lexer("foo // this is a comment\nbar /* block\ncomment */ baz");
    assert_token_value(&lex, TOKEN_IDENT, "foo", "foo");
    assert_token_value(&lex, TOKEN_IDENT, "bar", "bar after line comment");
    assert_token_value(&lex, TOKEN_IDENT, "baz", "baz after block comment");
}

static void test_complex_function(void) {
    printf("  complex function...\n");
    Lexer lex = make_lexer("fn add(x: i32, y: i32) -> i32 {\n  return x + y\n}");
    assert_token(&lex, TOKEN_FN, "fn");
    assert_token_value(&lex, TOKEN_IDENT, "add", "add");
    assert_token(&lex, TOKEN_LPAREN, "(");
    assert_token_value(&lex, TOKEN_IDENT, "x", "x");
    assert_token(&lex, TOKEN_COLON, ":");
    assert_token_value(&lex, TOKEN_IDENT, "i32", "i32");
    assert_token(&lex, TOKEN_COMMA, ",");
    assert_token_value(&lex, TOKEN_IDENT, "y", "y");
    assert_token(&lex, TOKEN_COLON, ":");
    assert_token_value(&lex, TOKEN_IDENT, "i32", "i32");
    assert_token(&lex, TOKEN_RPAREN, ")");
    assert_token(&lex, TOKEN_ARROW, "->");
    assert_token_value(&lex, TOKEN_IDENT, "i32", "i32 return");
    assert_token(&lex, TOKEN_LBRACE, "{");
    assert_token(&lex, TOKEN_RETURN, "return");
    assert_token_value(&lex, TOKEN_IDENT, "x", "x");
    assert_token(&lex, TOKEN_PLUS, "+");
    assert_token_value(&lex, TOKEN_IDENT, "y", "y");
    assert_token(&lex, TOKEN_RBRACE, "}");
}

static void test_agent_chain(void) {
    printf("  agent chain...\n");
    Lexer lex = make_lexer("agent |=> search_web(query) |=> summarize()");
    assert_token(&lex, TOKEN_AGENT, "agent");
    assert_token(&lex, TOKEN_PIPE_ARROW, "|=>");
    assert_token_value(&lex, TOKEN_IDENT, "search_web", "search_web");
    assert_token(&lex, TOKEN_LPAREN, "(");
    assert_token_value(&lex, TOKEN_IDENT, "query", "query");
    assert_token(&lex, TOKEN_RPAREN, ")");
    assert_token(&lex, TOKEN_PIPE_ARROW, "|=>");
    assert_token_value(&lex, TOKEN_IDENT, "summarize", "summarize");
    assert_token(&lex, TOKEN_LPAREN, "(");
    assert_token(&lex, TOKEN_RPAREN, ")");
}

static void test_packet_decl(void) {
    printf("  packet declaration...\n");
    Lexer lex = make_lexer("packet main { import io }");
    assert_token(&lex, TOKEN_PACKET, "packet");
    assert_token_value(&lex, TOKEN_IDENT, "main", "main");
    assert_token(&lex, TOKEN_LBRACE, "{");
    assert_token(&lex, TOKEN_IMPORT, "import");
    assert_token_value(&lex, TOKEN_IDENT, "io", "io");
    assert_token(&lex, TOKEN_RBRACE, "}");
}

int main(void) {
    printf("Running lexer tests...\n");
    test_keywords();
    test_operators();
    test_delimiters();
    test_integer_literals();
    test_float_literals();
    test_string_literals();
    test_char_literals();
    test_identifiers();
    test_comments();
    test_complex_function();
    test_agent_chain();
    test_packet_decl();
    printf("\nResults: %d/%d passed", pass_count, test_count);
    if (fail_count > 0) {
        printf(" (%d FAILED)\n", fail_count);
        return 1;
    }
    printf(" — all OK\n");
    return 0;
}
