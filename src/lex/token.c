#include <stdio.h>
#include <stdlib.h>

#include <inc/token.h>
#include <inc/util.h>
#include <inc/dynarr.h>

enum {
	TOKEN_INVALID = 0,
	TOKEN_NO_EXTRA_MEM = 1,
	TOKEN_EXTRA_MEM = 2,
};

static int token_need_extra_mem[] = {
#define DEF(tok) [tok] = TOKEN_NO_EXTRA_MEM,
#define DEFV(tok, val) DEF(tok)
#define DEFM(tok) [tok] = TOKEN_EXTRA_MEM,
#include <lex/token.def>
#undef DEFV
#undef DEF
#undef DEFM
	[TOK_TOTAL_NUM] = TOKEN_INVALID,
};

union token *token_shallow_dup(union token *inp) {
	union token *out = mallocz(sizeof(*out));
	*out = *inp;
	return out;
}

union token *token_deep_dup(union token *inp) {
	assert((unsigned) inp->tok_tag <= TOK_TOTAL_NUM && token_need_extra_mem[inp->tok_tag] != TOKEN_INVALID);
	union token *out = mallocz(sizeof(*out));
	*out = *inp;

	if (token_need_extra_mem[inp->tok_tag] == TOKEN_EXTRA_MEM) {
		switch (inp->tok_tag) {
		case TOK_TYPE_NAME:
		case TOK_IDENTIFIER:
			out->id.s = strdup(inp->id.s);
			break;
		case TOK_STRING_LITERAL:
			out->str.s = strdup(inp->str.s);
			break;
		default:
			panic("not supported %s", token_tag_str(inp->tok_tag));
		}
	}
	return out;
}

void token_destroy(union token token) {
	unsigned int tag = token.tok_tag;
	if (tag >= TOK_TOTAL_NUM) {
		panic("invalid token %d", tag);
	}
	int flag = token_need_extra_mem[tag];
	if (flag == TOKEN_INVALID) {
		panic("invalid token %d", tag);
	} 
	if (flag == TOKEN_NO_EXTRA_MEM) {
		return;
	}
	assert(flag == TOKEN_EXTRA_MEM);
	switch (token.tok_tag) {
	case TOK_TYPE_NAME:
	case TOK_IDENTIFIER:
		free(token.id.s);
		break;
	case TOK_STRING_LITERAL:
		free(token.str.s);
		break;
	default:
		panic("token_destroy %s ni", token_tag_str(tag));
		break;
	}
}

void token_list_dump(struct dynarr *darr) {
	DYNARR_FOREACH_PLAIN_BEGIN(darr, union token *, each);
		token_dump(*each);
	DYNARR_FOREACH_END();
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
#define DEFM(tok) DEF(tok)
#include <lex/token.def>
#undef DEFV
#undef DEF
#undef DEFM
	[TOK_TOTAL_NUM] = NULL,
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

