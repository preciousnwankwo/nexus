#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void parser_error(Parser *p, const char *msg) {
    Token t = p->lexer->current;
    fprintf(stderr, "%s:%d:%d: error: %s\n",
            t.loc.file ? t.loc.file : "<input>",
            t.loc.line, t.loc.col, msg);
    p->had_error = 1;
}

static Token parser_peek(Parser *p) {
    return p->lexer->current;
}

static Token parser_advance(Parser *p) {
    return lexer_next(p->lexer);
}

static Token parser_expect(Parser *p, TokenKind kind) {
    Token t = parser_peek(p);
    if (t.kind != kind) {
        fprintf(stderr, "%s:%d:%d: expected %s, got %s\n",
                t.loc.file ? t.loc.file : "<input>",
                t.loc.line, t.loc.col,
                token_kind_name(kind), token_kind_name(t.kind));
        p->had_error = 1;
        return t;
    }
    return parser_advance(p);
}

static int parser_match(Parser *p, TokenKind kind) {
    if (parser_peek(p).kind == kind) {
        parser_advance(p);
        return 1;
    }
    return 0;
}

int prec(TokenKind kind) {
    switch (kind) {
        case TOKEN_OR: return 1;
        case TOKEN_AND: return 2;
        case TOKEN_PIPE: return 3;
        case TOKEN_CARET: return 4;
        case TOKEN_AMP: return 5;
        case TOKEN_EQEQ: case TOKEN_NEQ: return 6;
        case TOKEN_LT: case TOKEN_GT: case TOKEN_LE: case TOKEN_GE: return 7;
        case TOKEN_SHL: case TOKEN_SHR: return 8;
        case TOKEN_PLUS: case TOKEN_MINUS: return 9;
        case TOKEN_STAR: case TOKEN_SLASH: case TOKEN_PERCENT: return 10;
        default: return -1;
    }
}

static int is_unary_op(TokenKind kind) {
    return kind == TOKEN_MINUS || kind == TOKEN_BANG || kind == TOKEN_TILDE;
}

void parser_init(Parser *p, Lexer *lex, Arena *arena, StringTable *strings) {
    p->lexer = lex;
    p->arena = arena;
    p->strings = strings;
    p->had_error = 0;
}

AstNode *parse_expression(Parser *p) {
    return parse_binary(p, 0);
}

AstNode *parse_binary(Parser *p, int min_prec) {
    AstNode *left = parse_unary(p);

    for (;;) {
        Token op = parser_peek(p);
        int op_prec = prec(op.kind);
        if (op_prec < min_prec) break;

        parser_advance(p);
        AstNode *right = parse_binary(p, op_prec + 1);
        left = ast_new_binary(op.kind, left, right, p->arena);
        left->loc = left->data.binary.left->loc;
    }

    return left;
}

AstNode *parse_unary(Parser *p) {
    Token t = parser_peek(p);

    if (is_unary_op(t.kind)) {
        parser_advance(p);
        AstNode *operand = parse_unary(p);
        return ast_new_unary(t.kind, operand, p->arena);
    }

    return parse_primary(p);
}

AstNode *parse_primary(Parser *p) {
    Token t = parser_peek(p);

    switch (t.kind) {
        case TOKEN_INT_LIT: {
            parser_advance(p);
            AstNode *node = ast_new_lit(NODE_INT_LIT, t.value, t.value_len, p->arena);
            node->loc = t.loc;
            return node;
        }
        case TOKEN_FLOAT_LIT: {
            parser_advance(p);
            AstNode *node = ast_new_lit(NODE_FLOAT_LIT, t.value, t.value_len, p->arena);
            node->loc = t.loc;
            return node;
        }
        case TOKEN_STRING_LIT: {
            parser_advance(p);
            AstNode *node = ast_new_lit(NODE_STRING_LIT, t.value, t.value_len, p->arena);
            node->loc = t.loc;
            return node;
        }
        case TOKEN_CHAR_LIT: {
            parser_advance(p);
            AstNode *node = ast_new_lit(NODE_CHAR_LIT, t.value, 1, p->arena);
            node->loc = t.loc;
            return node;
        }
        case TOKEN_TRUE: case TOKEN_FALSE: {
            parser_advance(p);
            AstNode *node = ast_new(NODE_BOOL_LIT, p->arena);
            node->data.bool_lit.value = (t.kind == TOKEN_TRUE) ? 1 : 0;
            node->loc = t.loc;
            return node;
        }
        case TOKEN_NIL: {
            parser_advance(p);
            AstNode *node = ast_new(NODE_NIL_LIT, p->arena);
            node->loc = t.loc;
            return node;
        }
        case TOKEN_IDENT: {
            parser_advance(p);
            AstNode *node = ast_new_identifier(t.value, p->arena);
            node->loc = t.loc;

            if (parser_peek(p).kind == TOKEN_LPAREN) {
                node = parse_call(p, node);
            } else if (parser_peek(p).kind == TOKEN_LBRACKET) {
                node = parse_index(p, node);
            } else if (parser_peek(p).kind == TOKEN_DOT) {
                parser_advance(p);
                Token field = parser_expect(p, TOKEN_IDENT);
                AstNode *fa = ast_new(NODE_FIELD_ACCESS, p->arena);
                fa->data.field_access.object = node;
                fa->data.field_access.field = field.value;
                fa->loc = node->loc;
                node = fa;
            }
            return node;
        }
        case TOKEN_LPAREN: {
            parser_advance(p);
            AstNode *expr = parse_expression(p);
            parser_expect(p, TOKEN_RPAREN);
            return expr;
        }
        case TOKEN_LBRACKET: {
            parser_advance(p);
            AstNode *node = ast_new(NODE_ARRAY_LIT, p->arena);
            node->loc = t.loc;

            size_t cap = 8;
            node->data.array_lit.elements = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            node->data.array_lit.element_count = 0;

            if (parser_peek(p).kind != TOKEN_RBRACKET) {
                do {
                    if (node->data.array_lit.element_count >= cap) {
                        cap *= 2;
                        AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                        memcpy(new_buf, node->data.array_lit.elements,
                               sizeof(AstNode *) * node->data.array_lit.element_count);
                        node->data.array_lit.elements = new_buf;
                    }
                    node->data.array_lit.elements[node->data.array_lit.element_count++] =
                        parse_expression(p);
                } while (parser_match(p, TOKEN_COMMA));
            }

            parser_expect(p, TOKEN_RBRACKET);
            return node;
        }
        case TOKEN_IF: {
            return parse_if_expr(p);
        }
        case TOKEN_MATCH: {
            return parse_match_expr(p);
        }
        case TOKEN_UNSAFE: {
            parser_advance(p);
            AstNode *expr = parse_expression(p);
            AstNode *node = ast_new(NODE_UNSAFE_EXPR, p->arena);
            node->data.unsafe_expr.expr = expr;
            node->loc = t.loc;
            return node;
        }
        case TOKEN_AGENT: {
            parser_advance(p);
            AstNode *node = ast_new(NODE_AGENT_EXPR, p->arena);
            node->loc = t.loc;
            size_t cap = 4;
            node->data.agent_expr.args = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            node->data.agent_expr.arg_count = 0;

            while (parser_match(p, TOKEN_PIPE_ARROW)) {
                AstNode *arg = parse_primary(p);
                if (node->data.agent_expr.arg_count + 1 >= cap) {
                    cap *= 2;
                    AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                    memcpy(new_buf, node->data.agent_expr.args,
                           sizeof(AstNode *) * node->data.agent_expr.arg_count);
                    node->data.agent_expr.args = new_buf;
                }
                node->data.agent_expr.args[node->data.agent_expr.arg_count++] = arg;
            }
            return node;
        }
        case TOKEN_PARALLEL: {
            parser_advance(p);
            AstNode *node = ast_new(NODE_PARALLEL_EXPR, p->arena);
            node->loc = t.loc;

            parser_expect(p, TOKEN_LBRACE);
            size_t cap = 4;
            node->data.parallel_expr.blocks = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            node->data.parallel_expr.block_count = 0;

            while (parser_peek(p).kind != TOKEN_RBRACE) {
                if (node->data.parallel_expr.block_count >= cap) {
                    cap *= 2;
                    AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                    memcpy(new_buf, node->data.parallel_expr.blocks,
                           sizeof(AstNode *) * node->data.parallel_expr.block_count);
                    node->data.parallel_expr.blocks = new_buf;
                }
                node->data.parallel_expr.blocks[node->data.parallel_expr.block_count++] =
                    parse_statement(p);
            }
            parser_expect(p, TOKEN_RBRACE);
            return node;
        }
        case TOKEN_AWAIT: {
            parser_advance(p);
            AstNode *expr = parse_expression(p);
            AstNode *node = ast_new(NODE_AWAIT_EXPR, p->arena);
            node->data.await_expr.expr = expr;
            node->loc = t.loc;
            return node;
        }
        default:
            parser_error(p, "unexpected token in expression");
            parser_advance(p);
            return ast_new(NODE_NIL_LIT, p->arena);
    }
}

AstNode *parse_call(Parser *p, AstNode *callee) {
    SourceLocation loc = callee->loc;
    parser_expect(p, TOKEN_LPAREN);

    size_t cap = 8;
    AstNode **args = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t arg_count = 0;

    if (parser_peek(p).kind != TOKEN_RPAREN) {
        do {
            if (arg_count >= cap) {
                cap *= 2;
                AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                memcpy(new_buf, args, sizeof(AstNode *) * arg_count);
                args = new_buf;
            }
            args[arg_count++] = parse_expression(p);
        } while (parser_match(p, TOKEN_COMMA));
    }

    parser_expect(p, TOKEN_RPAREN);

    AstNode *node = ast_new(NODE_CALL_EXPR, p->arena);
    node->data.call.callee = callee;
    node->data.call.args = args;
    node->data.call.arg_count = arg_count;
    node->loc = loc;
    return node;
}

AstNode *parse_index(Parser *p, AstNode *collection) {
    SourceLocation loc = collection->loc;
    parser_expect(p, TOKEN_LBRACKET);
    AstNode *index = parse_expression(p);
    parser_expect(p, TOKEN_RBRACKET);

    AstNode *node = ast_new(NODE_INDEX_EXPR, p->arena);
    node->data.index_expr.collection = collection;
    node->data.index_expr.index = index;
    node->loc = loc;
    return node;
}

AstNode *parse_type(Parser *p) {
    Token t = parser_peek(p);

    if (t.kind == TOKEN_STAR) {
        parser_advance(p);
        AstNode *inner = parse_type(p);
        AstNode *node = ast_new(NODE_TYPE_PTR, p->arena);
        node->data.type_ptr.pointee = inner;
        node->loc = t.loc;
        return node;
    }

    if (t.kind == TOKEN_AMP) {
        parser_advance(p);
        AstNode *inner = parse_type(p);
        AstNode *node = ast_new(NODE_TYPE_REF, p->arena);
        node->data.type_ref.referent = inner;
        node->loc = t.loc;
        return node;
    }

    if (t.kind == TOKEN_LBRACKET) {
        parser_advance(p);
        AstNode *elem = parse_type(p);
        if (parser_match(p, TOKEN_SEMICOLON)) {
            AstNode *size = parse_expression(p);
            parser_expect(p, TOKEN_RBRACKET);
            AstNode *node = ast_new(NODE_TYPE_ARRAY, p->arena);
            node->data.type_array.element_type = elem;
            node->data.type_array.size = size;
            node->loc = t.loc;
            return node;
        }
        parser_expect(p, TOKEN_RBRACKET);
        AstNode *node = ast_new(NODE_TYPE_SLICE, p->arena);
        node->data.type_slice.element_type = elem;
        node->loc = t.loc;
        return node;
    }

    if (t.kind == TOKEN_LPAREN) {
        parser_advance(p);
        size_t cap = 4;
        AstNode **types = arena_alloc(p->arena, sizeof(AstNode *) * cap);
        size_t count = 0;

        while (parser_peek(p).kind != TOKEN_RPAREN) {
            if (count >= cap) {
                cap *= 2;
                AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                memcpy(new_buf, types, sizeof(AstNode *) * count);
                types = new_buf;
            }
            types[count++] = parse_type(p);
            parser_match(p, TOKEN_COMMA);
        }
        parser_expect(p, TOKEN_RPAREN);

        if (count == 1) {
            return types[0];
        }
        AstNode *node = ast_new(NODE_TYPE_TUPLE, p->arena);
        node->data.type_tuple.element_types = types;
        node->data.type_tuple.element_count = count;
        node->loc = t.loc;
        return node;
    }

    if (t.kind == TOKEN_IDENT) {
        parser_advance(p);
        AstNode *node = ast_new(NODE_TYPE_PATH, p->arena);
        node->data.type_path.name = t.value;
        node->loc = t.loc;

        size_t cap = 4;
        node->data.type_path.generics = arena_alloc(p->arena, sizeof(AstNode *) * cap);
        node->data.type_path.generic_count = 0;

        if (parser_match(p, TOKEN_LT)) {
            do {
                if (node->data.type_path.generic_count >= cap) {
                    cap *= 2;
                    AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                    memcpy(new_buf, node->data.type_path.generics,
                           sizeof(AstNode *) * node->data.type_path.generic_count);
                    node->data.type_path.generics = new_buf;
                }
                node->data.type_path.generics[node->data.type_path.generic_count++] =
                    parse_type(p);
            } while (parser_match(p, TOKEN_COMMA));
            parser_expect(p, TOKEN_GT);
        }
        return node;
    }

    if (t.kind == TOKEN_FN) {
        parser_advance(p);
        parser_expect(p, TOKEN_LPAREN);

        size_t cap = 4;
        AstNode **param_types = arena_alloc(p->arena, sizeof(AstNode *) * cap);
        size_t param_count = 0;

        while (parser_peek(p).kind != TOKEN_RPAREN) {
            if (param_count >= cap) {
                cap *= 2;
                AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                memcpy(new_buf, param_types, sizeof(AstNode *) * param_count);
                param_types = new_buf;
            }
            param_types[param_count++] = parse_type(p);
            parser_match(p, TOKEN_COMMA);
        }
        parser_expect(p, TOKEN_RPAREN);

        AstNode *ret = NULL;
        if (parser_match(p, TOKEN_ARROW)) {
            ret = parse_type(p);
        }

        AstNode *node = ast_new(NODE_TYPE_FN, p->arena);
        node->data.type_fn.param_types = param_types;
        node->data.type_fn.param_count = param_count;
        node->data.type_fn.return_type = ret;
        node->loc = t.loc;
        return node;
    }

    parser_error(p, "expected type");
    parser_advance(p);
    return ast_new(NODE_NIL_LIT, p->arena);
}

AstNode *parse_let_stmt(Parser *p) {
    Token t = parser_expect(p, TOKEN_LET);
    int is_mut = parser_match(p, TOKEN_MUT);
    Token name = parser_expect(p, TOKEN_IDENT);

    AstNode *type = NULL;
    if (parser_match(p, TOKEN_COLON)) {
        type = parse_type(p);
    }

    AstNode *init = NULL;
    if (parser_match(p, TOKEN_EQ)) {
        init = parse_expression(p);
    }

    AstNode *node = ast_new(NODE_LET_STMT, p->arena);
    node->data.let_stmt.name = name.value;
    node->data.let_stmt.type = type;
    node->data.let_stmt.init = init;
    node->data.let_stmt.is_mut = is_mut;
    node->loc = t.loc;
    return node;
}

AstNode *parse_return_stmt(Parser *p) {
    Token t = parser_expect(p, TOKEN_RETURN);
    AstNode *value = NULL;
    if (parser_peek(p).kind != TOKEN_RBRACE &&
        parser_peek(p).kind != TOKEN_SEMICOLON &&
        parser_peek(p).kind != TOKEN_EOF) {
        value = parse_expression(p);
    }

    AstNode *node = ast_new(NODE_RETURN_STMT, p->arena);
    node->data.return_stmt.value = value;
    node->loc = t.loc;
    return node;
}

AstNode *parse_if_expr(Parser *p) {
    Token t = parser_expect(p, TOKEN_IF);
    AstNode *cond = parse_expression(p);
    AstNode *then_block = parse_block(p);

    AstNode *else_block = NULL;
    if (parser_match(p, TOKEN_ELSE)) {
        if (parser_peek(p).kind == TOKEN_IF) {
            else_block = parse_if_expr(p);
        } else {
            else_block = parse_block(p);
        }
    }

    AstNode *node = ast_new(NODE_IF_EXPR, p->arena);
    node->data.if_expr.cond = cond;
    node->data.if_expr.then_block = then_block;
    node->data.if_expr.else_block = else_block;
    node->loc = t.loc;
    return node;
}

AstNode *parse_match_arm(Parser *p) {
    AstNode *pattern = parse_expression(p);
    parser_expect(p, TOKEN_FAT_ARROW);
    AstNode *body = parse_expression(p);

    AstNode *node = ast_new(NODE_MATCH_ARM, p->arena);
    node->data.match_arm.pattern = pattern;
    node->data.match_arm.body = body;
    node->loc = pattern->loc;
    return node;
}

AstNode *parse_match_expr(Parser *p) {
    Token t = parser_expect(p, TOKEN_MATCH);
    AstNode *value = parse_expression(p);

    parser_expect(p, TOKEN_LBRACE);

    size_t cap = 8;
    AstNode **arms = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t arm_count = 0;

    while (parser_peek(p).kind != TOKEN_RBRACE) {
        if (arm_count >= cap) {
            cap *= 2;
            AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            memcpy(new_buf, arms, sizeof(AstNode *) * arm_count);
            arms = new_buf;
        }
        arms[arm_count++] = parse_match_arm(p);
    }

    parser_expect(p, TOKEN_RBRACE);

    AstNode *node = ast_new(NODE_MATCH_EXPR, p->arena);
    node->data.match_expr.value = value;
    node->data.match_expr.arms = arms;
    node->data.match_expr.arm_count = arm_count;
    node->loc = t.loc;
    return node;
}

AstNode *parse_block(Parser *p) {
    Token t = parser_expect(p, TOKEN_LBRACE);

    size_t cap = 16;
    AstNode **stmts = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t stmt_count = 0;

    while (parser_peek(p).kind != TOKEN_RBRACE && parser_peek(p).kind != TOKEN_EOF) {
        if (stmt_count >= cap) {
            cap *= 2;
            AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            memcpy(new_buf, stmts, sizeof(AstNode *) * stmt_count);
            stmts = new_buf;
        }
        stmts[stmt_count++] = parse_statement(p);
    }

    parser_expect(p, TOKEN_RBRACE);

    AstNode *node = ast_new(NODE_BLOCK, p->arena);
    node->data.block.items = stmts;
    node->data.block.item_count = stmt_count;
    node->loc = t.loc;
    return node;
}

AstNode *parse_param(Parser *p) {
    Token name = parser_expect(p, TOKEN_IDENT);
    parser_expect(p, TOKEN_COLON);
    AstNode *type = parse_type(p);

    AstNode *node = ast_new(NODE_PARAM, p->arena);
    node->data.param.name = name.value;
    node->data.param.type = type;
    node->data.param.is_mut = 0;
    node->loc = name.loc;
    return node;
}

AstNode *parse_fn_decl(Parser *p) {
    Token t = parser_expect(p, TOKEN_FN);
    Token name = parser_expect(p, TOKEN_IDENT);

    parser_expect(p, TOKEN_LPAREN);

    size_t cap = 8;
    AstNode **params = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t param_count = 0;

    if (parser_peek(p).kind != TOKEN_RPAREN) {
        do {
            if (param_count >= cap) {
                cap *= 2;
                AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                memcpy(new_buf, params, sizeof(AstNode *) * param_count);
                params = new_buf;
            }
            params[param_count++] = parse_param(p);
        } while (parser_match(p, TOKEN_COMMA));
    }

    parser_expect(p, TOKEN_RPAREN);

    AstNode *return_type = NULL;
    if (parser_match(p, TOKEN_ARROW)) {
        return_type = parse_type(p);
    }

    AstNode *body = parse_block(p);

    AstNode *node = ast_new(NODE_FN_DECL, p->arena);
    node->data.fn_decl.name = name.value;
    node->data.fn_decl.params = params;
    node->data.fn_decl.param_count = param_count;
    node->data.fn_decl.return_type = return_type;
    node->data.fn_decl.body = body;
    node->loc = t.loc;
    return node;
}

AstNode *parse_field(Parser *p) {
    Token name = parser_expect(p, TOKEN_IDENT);
    parser_expect(p, TOKEN_COLON);
    AstNode *type = parse_type(p);

    AstNode *node = ast_new(NODE_FIELD, p->arena);
    node->data.field.name = name.value;
    node->data.field.type = type;
    node->loc = name.loc;
    return node;
}

AstNode *parse_struct_decl(Parser *p) {
    Token t = parser_expect(p, TOKEN_STRUCT);
    Token name = parser_expect(p, TOKEN_IDENT);
    parser_expect(p, TOKEN_LBRACE);

    size_t cap = 8;
    AstNode **fields = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t field_count = 0;

    while (parser_peek(p).kind != TOKEN_RBRACE) {
        if (field_count >= cap) {
            cap *= 2;
            AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            memcpy(new_buf, fields, sizeof(AstNode *) * field_count);
            fields = new_buf;
        }
        fields[field_count++] = parse_field(p);
    }

    parser_expect(p, TOKEN_RBRACE);

    AstNode *node = ast_new(NODE_STRUCT_DECL, p->arena);
    node->data.struct_decl.name = name.value;
    node->data.struct_decl.fields = fields;
    node->data.struct_decl.field_count = field_count;
    node->loc = t.loc;
    return node;
}

AstNode *parse_variant(Parser *p) {
    Token name = parser_expect(p, TOKEN_IDENT);

    size_t cap = 4;
    AstNode **fields = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t field_count = 0;

    if (parser_match(p, TOKEN_LPAREN)) {
        if (parser_peek(p).kind != TOKEN_RPAREN) {
            do {
                if (field_count >= cap) {
                    cap *= 2;
                    AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
                    memcpy(new_buf, fields, sizeof(AstNode *) * field_count);
                    fields = new_buf;
                }
                Token fname = parser_expect(p, TOKEN_IDENT);
                parser_expect(p, TOKEN_COLON);
                AstNode *type = parse_type(p);

                AstNode *vf = ast_new(NODE_VARIANT_FIELD, p->arena);
                vf->data.variant_field.name = fname.value;
                vf->data.variant_field.type = type;
                vf->loc = fname.loc;
                fields[field_count++] = vf;
            } while (parser_match(p, TOKEN_COMMA));
        }
        parser_expect(p, TOKEN_RPAREN);
    }

    AstNode *node = ast_new(NODE_VARIANT, p->arena);
    node->data.variant.name = name.value;
    node->data.variant.fields = fields;
    node->data.variant.field_count = field_count;
    node->loc = name.loc;
    return node;
}

AstNode *parse_enum_decl(Parser *p) {
    Token t = parser_expect(p, TOKEN_ENUM);
    Token name = parser_expect(p, TOKEN_IDENT);
    parser_expect(p, TOKEN_LBRACE);

    size_t cap = 8;
    AstNode **variants = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t variant_count = 0;

    while (parser_peek(p).kind != TOKEN_RBRACE) {
        if (variant_count >= cap) {
            cap *= 2;
            AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            memcpy(new_buf, variants, sizeof(AstNode *) * variant_count);
            variants = new_buf;
        }
        variants[variant_count++] = parse_variant(p);
        parser_match(p, TOKEN_COMMA);
    }

    parser_expect(p, TOKEN_RBRACE);

    AstNode *node = ast_new(NODE_ENUM_DECL, p->arena);
    node->data.enum_decl.name = name.value;
    node->data.enum_decl.variants = variants;
    node->data.enum_decl.variant_count = variant_count;
    node->loc = t.loc;
    return node;
}

AstNode *parse_import_decl(Parser *p) {
    Token t = parser_expect(p, TOKEN_IMPORT);
    Token name = parser_expect(p, TOKEN_IDENT);

    AstNode *node = ast_new(NODE_IMPORT_DECL, p->arena);
    node->data.type_path.name = name.value;
    node->loc = t.loc;
    return node;
}

AstNode *parse_packet_decl(Parser *p) {
    Token t = parser_expect(p, TOKEN_PACKET);
    Token name = parser_expect(p, TOKEN_IDENT);

    parser_expect(p, TOKEN_LBRACE);

    size_t cap = 8;
    AstNode **stmts = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t stmt_count = 0;

    while (parser_peek(p).kind != TOKEN_RBRACE) {
        if (stmt_count >= cap) {
            cap *= 2;
            AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            memcpy(new_buf, stmts, sizeof(AstNode *) * stmt_count);
            stmts = new_buf;
        }
        stmts[stmt_count++] = parse_statement(p);
    }

    parser_expect(p, TOKEN_RBRACE);

    AstNode *node = ast_new(NODE_PACKET_DECL, p->arena);
    node->data.fn_decl.name = name.value;
    node->data.fn_decl.params = stmts;
    node->data.fn_decl.param_count = stmt_count;
    node->loc = t.loc;
    return node;
}

AstNode *parse_statement(Parser *p) {
    Token t = parser_peek(p);

    switch (t.kind) {
        case TOKEN_LET: return parse_let_stmt(p);
        case TOKEN_RETURN: return parse_return_stmt(p);
        case TOKEN_FN: return parse_fn_decl(p);
        case TOKEN_STRUCT: return parse_struct_decl(p);
        case TOKEN_ENUM: return parse_enum_decl(p);
        case TOKEN_IMPORT: return parse_import_decl(p);
        case TOKEN_PACKET: return parse_packet_decl(p);
        default: {
            AstNode *expr = parse_expression(p);
            AstNode *node = ast_new(NODE_EXPR_STMT, p->arena);
            node->data.expr_stmt.expr = expr;
            node->loc = t.loc;
            return node;
        }
    }
}

AstNode *parse_program(Parser *p) {
    size_t cap = 16;
    AstNode **stmts = arena_alloc(p->arena, sizeof(AstNode *) * cap);
    size_t stmt_count = 0;

    while (parser_peek(p).kind != TOKEN_EOF) {
        if (stmt_count >= cap) {
            cap *= 2;
            AstNode **new_buf = arena_alloc(p->arena, sizeof(AstNode *) * cap);
            memcpy(new_buf, stmts, sizeof(AstNode *) * stmt_count);
            stmts = new_buf;
        }
        stmts[stmt_count++] = parse_statement(p);
    }

    AstNode *node = ast_new(NODE_PROGRAM, p->arena);
    node->data.program.stmts = stmts;
    node->data.program.stmt_count = stmt_count;
    return node;
}

AstNode *parser_parse(Parser *p) {
    AstNode *program = parse_program(p);
    if (p->had_error) {
        return NULL;
    }
    return program;
}
