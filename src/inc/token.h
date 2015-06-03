#ifndef _INC_TOKEN_H
#define _INC_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

union token {
	int token_tag;

	struct {
		int token_tag;
		char *s;
	} id_token;
};

enum {
	TOK_LPAREN = '(',
	TOK_RPAREN = ')',
	TOK_LBRACE = '{',
	TOK_RBRACE = '}',
	TOK_COMMA = ',',
	TOK_SEMICOLON = ';',
	TOK_STRING_LITERAL = '"',

	TOK_INT = 256,
	TOK_IDENTIFIER,

	TOK_EOF,
	TOK_UNDEF,
};

void token_destroy(union token token);
void token_dump(union token token);

#ifdef __cplusplus
}
#endif

#endif
