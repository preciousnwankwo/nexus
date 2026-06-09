#ifndef NEXUS_LEXER_H
#define NEXUS_LEXER_H

#include <stddef.h>
#include "arena.h"
#include "string_table.h"

typedef enum {
    TOKEN_EOF,

    TOKEN_IDENT,
    TOKEN_INT_LIT,
    TOKEN_FLOAT_LIT,
    TOKEN_STRING_LIT,
    TOKEN_CHAR_LIT,

    // Keywords
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_MATCH,
    TOKEN_RETURN,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IMPORT,
    TOKEN_EXPORT,
    TOKEN_PACKET,
    TOKEN_PARALLEL,
    TOKEN_ASYNC,
    TOKEN_AWAIT,
    TOKEN_AGENT,
    TOKEN_UNSAFE,
    TOKEN_ARENA,
    TOKEN_OWN,
    TOKEN_BORROW,
    TOKEN_REF,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_XOR,
    TOKEN_SHL,
    TOKEN_SHR,
    TOKEN_MOD,
    TOKEN_AS,
    TOKEN_TYPE,
    TOKEN_SELF,
    TOKEN_SUPER,
    TOKEN_WILDCARD,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_EQ,
    TOKEN_EQEQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AMP,
    TOKEN_PIPE,
    TOKEN_CARET,
    TOKEN_TILDE,
    TOKEN_BANG,
    TOKEN_ARROW,       // ->
    TOKEN_FAT_ARROW,   // =>
    TOKEN_PIPE_ARROW,  // |=> (agent chain)
    TOKEN_COLON,       // :
    TOKEN_COLON_COLON, // ::
    TOKEN_DOT,         // .
    TOKEN_DOT_DOT,     // ..
    TOKEN_DOT_DOT_DOT, // ...

    // Delimiters
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_AT,         // @
    TOKEN_HASH,       // #
    TOKEN_DOLLAR,     // $
    TOKEN_QUESTION,   // ?

    // Special
    TOKEN_NEWLINE,
    TOKEN_ERROR,
} TokenKind;

typedef struct {
    const char *file;
    int line;
    int col;
} SourceLocation;

typedef struct {
    TokenKind kind;
    const char *value;   // Interned string for idents/keywords, raw text for literals
    size_t value_len;
    SourceLocation loc;
} Token;

typedef struct {
    const char *source;
    size_t source_len;
    size_t pos;
    int line;
    int col;
    Arena *arena;
    StringTable *strings;
    Token current;
    Token peek;
    int has_peek;
} Lexer;

void lexer_init(Lexer *lex, const char *source, size_t source_len, Arena *arena, StringTable *strings);
Token lexer_next(Lexer *lex);
Token lexer_peek(Lexer *lex);
Token lexer_expect(Lexer *lex, TokenKind kind);
int lexer_match(Lexer *lex, TokenKind kind);
const char *token_kind_name(TokenKind kind);

#endif
