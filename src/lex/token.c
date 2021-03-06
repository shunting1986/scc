#include <stdio.h>
#include <stdlib.h>

#include <inc/token.h>
#include <inc/util.h>
#include <inc/dynarr.h>
#include <inc/cgc.h>
#include <inc/keyword.h>

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
	case TOK_TYPE_NAME:
		fprintf(stderr, "[TYPE_NAME]: %s\n", token.id.s);
		break;
	case TOK_IDENTIFIER: 
		fprintf(stderr, "[id]: %s\n", token.id.s);
		break;
	case TOK_STRING_LITERAL:
		fprintf(stderr, "[string_literal] %s\n", token.str.s);
		break;
	case TOK_CONSTANT_VALUE:
		fprintf(stderr, "[const_val] %d\n", token.const_val.ival);
		break;
	default:
		fprintf(stderr, "'%s'\n", token_tag_str(token.tok_tag));
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

bool const_token_is_nonzero(union token tok) {
	assert(tok.tok_tag == TOK_CONSTANT_VALUE);
	if (tok.const_val.flags & CONST_VAL_TOK_INTEGER) {
		return tok.const_val.ival != 0;
	} else if (tok.const_val.flags & CONST_VAL_TOK_FLOAT) {
		return tok.const_val.fval != 0;
	} else if (tok.const_val.flags & CONST_VAL_TOK_LONG_LONG) {
		return tok.const_val.llval != 0;
	} else {
		panic("invalid const token");
	}
}

union token wrap_int_const_to_token(int val) {
	union token tok;
	tok.const_val.tok_tag = TOK_CONSTANT_VALUE;
	tok.const_val.flags = CONST_VAL_TOK_INTEGER;
	tok.const_val.ival = val;
	return tok;
}

union token wrap_ll_const_to_token(long long llval) {
	union token tok;
	tok.const_val.tok_tag = TOK_CONSTANT_VALUE;
	tok.const_val.flags = CONST_VAL_TOK_LONG_LONG;
	tok.const_val.llval = llval;
	return tok;
}

double const_token_to_double(union token tok) {
	assert(tok.tok_tag == TOK_CONSTANT_VALUE);
	int flags = tok.const_val.flags;

	if (flags & CONST_VAL_TOK_INTEGER) {
		return tok.const_val.ival;
	} else if (flags & CONST_VAL_TOK_LONG_LONG) {
		return tok.const_val.llval;
	} else if (flags & CONST_VAL_TOK_FLOAT) {
		return tok.const_val.fval;
	} else {
		panic("invalid const token");
	}
}

union token wrap_to_simple_token(int tag) {
	union token tok;
	tok.tok_tag = tag;
	return tok;
}

union token wrap_to_str_literal_token(char *s) {
	union token tok;
	tok.str.tok_tag = TOK_STRING_LITERAL;
	tok.str.s = s;
	return tok;
}

void token_list_to_cstr(struct dynarr *toklist, struct cbuf *cbuf) {
	bool first = true;
	DYNARR_FOREACH_PLAIN_BEGIN(toklist, union token *, each);
		if (!first) {
			cbuf_add(cbuf, ' ');
		}
		first = false;
		token_to_cstr(*each, cbuf);
	DYNARR_FOREACH_END();
}

void const_val_to_cstr(union token tok, struct cbuf *cbuf) {
	char buf[256];
	if (tok.const_val.flags & CONST_VAL_TOK_INTEGER) {
		snprintf(buf, sizeof(buf), "%d", tok.const_val.ival);
		cbuf_add_str(cbuf, buf);
		return;
	}

	if (tok.const_val.flags & CONST_VAL_TOK_FLOAT) {
		snprintf(buf, sizeof(buf), "%f", tok.const_val.fval);
		cbuf_add_str(cbuf, buf);
		return;
	}

	if (tok.const_val.flags & CONST_VAL_TOK_LONG_LONG) {
		snprintf(buf, sizeof(buf), "%lldLL", tok.const_val.llval);
		cbuf_add_str(cbuf, buf);
		return;
	}
	panic("can not reach here");
}

// this is used to dump token back to C for pp "'#' arg"
void token_to_cstr(union token tok, struct cbuf *cbuf) {
	const char *s;
	if (tok.tok_tag >= 0 && tok.tok_tag < 256) {
		cbuf_add(cbuf, tok.tok_tag);
		return;
	}

	s = cgc_get_op_str_noabort(tok.tok_tag);
	if (s != NULL) {
		cbuf_add_str(cbuf, s);
		return;
	}

	s = keyword_str_noabort(tok.tok_tag);
	if (s != NULL) {
		cbuf_add_str(cbuf, s);
		return;
	}

	switch (tok.tok_tag) {
	case TOK_IDENTIFIER:
		cbuf_add_str(cbuf, tok.id.s);
		break;
	case TOK_TYPE_NAME:
		cbuf_add_str(cbuf, tok.id.s);
		break;
	case TOK_CONSTANT_VALUE:
		const_val_to_cstr(tok, cbuf);
		break;
	case TOK_STRING_LITERAL:
		cbuf_add(cbuf, '\"');
		cbuf_add(cbuf, '\"');
		cbuf_add_str(cbuf, tok.str.s);
		break;
	default:	
		panic("ni %s", token_tag_str(tok.tok_tag));
	}
}

const char *cmp_tok_to_jmp_suffix(int tok_tag, bool unsig) {
	switch (tok_tag) {
	case TOK_EQ:
		return "e";
	case TOK_NE:
		return "ne";
	case TOK_LT:
		return unsig ? "b" : "l";
	case TOK_GT:
		return unsig ? "a" : "g";
	case TOK_LE:
		return unsig ? "be" : "le";
	case TOK_GE:
		return unsig ? "ae" : "ge";
	default:
		panic("invalid cmp tok");
	}
}

bool is_cmp_tok(int tok_tag) {
	return tok_tag == TOK_EQ || tok_tag == TOK_NE || tok_tag == TOK_LT 
		|| tok_tag == TOK_LE || tok_tag == TOK_GT || tok_tag == TOK_GE;
}

int get_arith_from_complex_assign(int op) {
	switch (op) {
	case TOK_MUL_ASSIGN: return TOK_STAR;
	case TOK_DIV_ASSIGN: return TOK_DIV;
	case TOK_MOD_ASSIGN: return TOK_MOD;
	case TOK_ADD_ASSIGN: return TOK_ADD;
	case TOK_SUB_ASSIGN: return TOK_SUB;
	case TOK_LSHIFT_ASSIGN: return TOK_LSHIFT;
	case TOK_RSHIFT_ASSIGN: return TOK_RSHIFT;
	case TOK_AND_ASSIGN: return TOK_AMPERSAND;
	case TOK_OR_ASSIGN: return TOK_VERT_BAR;
	case TOK_XOR_ASSIGN: return TOK_XOR;
	default:
		panic("invalid op %s", token_tag_str(op));
	}
}


