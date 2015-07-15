#include <stdio.h>
#include <stdlib.h>

#include <inc/token.h>
#include <inc/util.h>

void token_destroy(union token token) {
	switch (token.tok_tag) {
	case TOK_UNDEF: // special rule for undef
	case TOK_INT: case TOK_LPAREN: case TOK_RPAREN: case TOK_LBRACE:
	case TOK_RBRACE: case TOK_COMMA: case TOK_SEMICOLON: case TOK_AMPERSAND:
	case TOK_ASSIGN: case TOK_ADD: case TOK_RETURN: case TOK_CONSTANT_VALUE:
		break;
	case TOK_IDENTIFIER:
		free(token.id.s);
		break;
	case TOK_STRING_LITERAL:
		free(token.str.s);
		break;
	default:
		panic("token_destroy %d ni", token.tok_tag);
		break;
	}
}

void token_dump(union token token) {
	switch (token.tok_tag) {
	case TOK_IDENTIFIER:
		printf("[id]: %s\n", token.id.s);
		break;
	case TOK_STRING_LITERAL:
		printf("[string_literal] %s\n", token.str.s);
		break;
	case TOK_CONSTANT_VALUE:
		printf("[const_val] %d\n", token.const_val.ival);
		break;
	default:
		printf("'%s'\n", token_tag_str(token.tok_tag));
		break;
	}
}

static const char *token_tag_str_list[] = {
#define DEF(tok) [tok] = #tok,
#define DEFV(tok, val) DEF(tok)
#include <lex/token.def>
#undef DEFV
#undef DEF
};

const char *token_tag_str(int tok_tag) {
	if (tok_tag < 0 || tok_tag >= TOK_TOTAL_NUM || token_tag_str_list[tok_tag] == NULL) {
		panic("Invalid token tag: %d", tok_tag);
	}
	return token_tag_str_list[tok_tag];
}

union token wrap_int_const_to_token(int val) {
	union token tok;
	tok.const_val.tok_tag = TOK_CONSTANT_VALUE;
	tok.const_val.flags = CONST_VAL_TOK_INTEGER;
	tok.const_val.ival = val;
	return tok;
}

