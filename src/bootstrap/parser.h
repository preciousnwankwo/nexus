#ifndef NEXUS_PARSER_H
#define NEXUS_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "arena.h"

typedef struct {
    Lexer *lexer;
    Arena *arena;
    StringTable *strings;
    int had_error;
} Parser;

void parser_init(Parser *p, Lexer *lex, Arena *arena, StringTable *strings);
AstNode *parser_parse(Parser *p);

AstNode *parse_program(Parser *p);
AstNode *parse_statement(Parser *p);
AstNode *parse_block(Parser *p);
AstNode *parse_expression(Parser *p);
AstNode *parse_type(Parser *p);

AstNode *parse_fn_decl(Parser *p);
AstNode *parse_struct_decl(Parser *p);
AstNode *parse_enum_decl(Parser *p);
AstNode *parse_import_decl(Parser *p);
AstNode *parse_packet_decl(Parser *p);

AstNode *parse_let_stmt(Parser *p);
AstNode *parse_return_stmt(Parser *p);
AstNode *parse_if_expr(Parser *p);
AstNode *parse_match_expr(Parser *p);

AstNode *parse_binary(Parser *p, int min_prec);
AstNode *parse_unary(Parser *p);
AstNode *parse_primary(Parser *p);
AstNode *parse_call(Parser *p, AstNode *callee);
AstNode *parse_index(Parser *p, AstNode *collection);

#endif
