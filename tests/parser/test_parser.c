#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/bootstrap/arena.h"
#include "../../src/bootstrap/string_table.h"
#include "../../src/bootstrap/lexer.h"
#include "../../src/bootstrap/ast.h"
#include "../../src/bootstrap/parser.h"

static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

static void assert_node(AstNode *node, NodeKind expected, const char *desc) {
    test_count++;
    if (node && node->kind == expected) {
        pass_count++;
    } else {
        fail_count++;
        fprintf(stderr, "FAIL: %s — expected %d, got %d\n", desc, expected, node ? node->kind : -1);
    }
}

static void assert_string(const char *actual, const char *expected, const char *desc) {
    test_count++;
    if (actual && expected && strcmp(actual, expected) == 0) {
        pass_count++;
    } else {
        fail_count++;
        fprintf(stderr, "FAIL: %s — expected '%s', got '%s'\n", desc, expected ? expected : "NULL", actual ? actual : "NULL");
    }
}

static void assert_int(int actual, int expected, const char *desc) {
    test_count++;
    if (actual == expected) {
        pass_count++;
    } else {
        fail_count++;
        fprintf(stderr, "FAIL: %s — expected %d, got %d\n", desc, expected, actual);
    }
}

static Parser make_parser(const char *source) {
    static Arena arena;
    static StringTable strings;
    arena_init(&arena);
    string_table_init(&strings, &arena);
    Lexer lex;
    lexer_init(&lex, source, strlen(source), &arena, &strings);
    Parser p;
    parser_init(&p, &lex, &arena, &strings);
    return p;
}

static void test_integer_literal(void) {
    printf("  integer literal...\n");
    Parser p = make_parser("42");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program node");
    assert_int(prog->data.program.stmt_count, 1, "one statement");
    assert_node(prog->data.program.stmts[0], NODE_EXPR_STMT, "expr stmt");
}

static void test_binary_expression(void) {
    printf("  binary expression...\n");
    Parser p = make_parser("1 + 2");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one stmt");
    AstNode *stmt = prog->data.program.stmts[0];
    assert_node(stmt, NODE_EXPR_STMT, "expr stmt");
}

static void test_precedence(void) {
    printf("  operator precedence...\n");
    Parser p = make_parser("1 + 2 * 3");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one stmt");
}

static void test_function_declaration(void) {
    printf("  function declaration...\n");
    Parser p = make_parser("fn add(x: i32, y: i32) -> i32 { return x + y }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one fn decl");
    assert_node(prog->data.program.stmts[0], NODE_FN_DECL, "fn decl");
    assert_string(prog->data.program.stmts[0]->data.fn_decl.name, "add", "fn name");
    assert_int(prog->data.program.stmts[0]->data.fn_decl.param_count, 2, "two params");
}

static void test_struct_declaration(void) {
    printf("  struct declaration...\n");
    Parser p = make_parser("struct Point { x: f64 y: f64 }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one struct decl");
    assert_node(prog->data.program.stmts[0], NODE_STRUCT_DECL, "struct decl");
    assert_string(prog->data.program.stmts[0]->data.struct_decl.name, "Point", "struct name");
    assert_int(prog->data.program.stmts[0]->data.struct_decl.field_count, 2, "two fields");
}

static void test_enum_declaration(void) {
    printf("  enum declaration...\n");
    Parser p = make_parser("enum Color { Red Green Blue }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one enum decl");
    assert_node(prog->data.program.stmts[0], NODE_ENUM_DECL, "enum decl");
    assert_string(prog->data.program.stmts[0]->data.enum_decl.name, "Color", "enum name");
    assert_int(prog->data.program.stmts[0]->data.enum_decl.variant_count, 3, "three variants");
}

static void test_let_statement(void) {
    printf("  let statement...\n");
    Parser p = make_parser("let x = 42");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one let stmt");
    assert_node(prog->data.program.stmts[0], NODE_LET_STMT, "let stmt");
    assert_string(prog->data.program.stmts[0]->data.let_stmt.name, "x", "var name");
    assert_int(prog->data.program.stmts[0]->data.let_stmt.is_mut, 0, "not mut");
}

static void test_mut_let_statement(void) {
    printf("  mut let statement...\n");
    Parser p = make_parser("let mut x = 42");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_node(prog->data.program.stmts[0], NODE_LET_STMT, "let stmt");
    assert_int(prog->data.program.stmts[0]->data.let_stmt.is_mut, 1, "is mut");
}

static void test_if_expression(void) {
    printf("  if expression...\n");
    Parser p = make_parser("if x { 1 } else { 0 }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one stmt");
}

static void test_match_expression(void) {
    printf("  match expression...\n");
    Parser p = make_parser("match x { Ok(v) => v Err(e) => 0 }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one stmt");
}

static void test_packet_declaration(void) {
    printf("  packet declaration...\n");
    Parser p = make_parser("packet main { import io }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one packet decl");
    assert_node(prog->data.program.stmts[0], NODE_PACKET_DECL, "packet decl");
    assert_string(prog->data.program.stmts[0]->data.fn_decl.name, "main", "packet name");
}

static void test_agent_chain(void) {
    printf("  agent chain...\n");
    Parser p = make_parser("agent |=> search(query) |=> summarize()");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one stmt");
}

static void test_parallel_block(void) {
    printf("  parallel block...\n");
    Parser p = make_parser("parallel { let a = 1 let b = 2 }");
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 1, "one stmt");
}

static void test_complex_program(void) {
    printf("  complex program...\n");
    Parser p = make_parser(
        "packet main {\n"
        "  import io\n"
        "}\n"
        "\n"
        "fn fibonacci(n: i32) -> i32 {\n"
        "  if n <= 1 {\n"
        "    return n\n"
        "  }\n"
        "  return fibonacci(n - 1) + fibonacci(n - 2)\n"
        "}\n"
        "\n"
        "struct Vector {\n"
        "  x: f64\n"
        "  y: f64\n"
        "  z: f64\n"
        "}\n"
    );
    AstNode *prog = parser_parse(&p);
    assert_node(prog, NODE_PROGRAM, "program");
    assert_int(prog->data.program.stmt_count, 3, "three top-level items");
    assert_node(prog->data.program.stmts[0], NODE_PACKET_DECL, "packet");
    assert_node(prog->data.program.stmts[1], NODE_FN_DECL, "fn");
    assert_node(prog->data.program.stmts[2], NODE_STRUCT_DECL, "struct");
}

int main(void) {
    printf("Running parser tests...\n");
    test_integer_literal();
    test_binary_expression();
    test_precedence();
    test_function_declaration();
    test_struct_declaration();
    test_enum_declaration();
    test_let_statement();
    test_mut_let_statement();
    test_if_expression();
    test_match_expression();
    test_packet_declaration();
    test_agent_chain();
    test_parallel_block();
    test_complex_program();
    printf("\nResults: %d/%d passed", pass_count, test_count);
    if (fail_count > 0) {
        printf(" (%d FAILED)\n", fail_count);
        return 1;
    }
    printf(" — all OK\n");
    return 0;
}
