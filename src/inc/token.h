#ifndef _INC_TOKEN_H
#define _INC_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

union token {
	int tok_tag;

	struct {
		int tok_tag;
		char *s;
	} id;

	struct {
		int tok_tag;
		union {
			int ival;
		};
	} const_val;
#define str id // for string literal, the s does not contains the quotes
};

enum {
	TOK_LPAREN = '(',
	TOK_RPAREN = ')',
	TOK_LBRACE = '{',
	TOK_RBRACE = '}',
	TOK_COMMA = ',',
	TOK_SEMICOLON = ';',
	TOK_AMPERSAND = '&',
	TOK_ASSIGN = '=',
	TOK_ADD = '+',

	TOK_INT = 256,
	TOK_RETURN,
	TOK_IDENTIFIER,
	TOK_STRING_LITERAL,
	TOK_CONSTANT_VALUE,

	TOK_EOF,
	TOK_UNDEF,
};

void token_destroy(union token token);
void token_dump(union token token);
char *token_tag_str(int tok_tag);

#ifdef __cplusplus
}
#endif

#endif
