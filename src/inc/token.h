#ifndef _INC_TOKEN_H
#define _INC_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

struct dynarr;

union token {
	int tok_tag;

	struct {
		int tok_tag;
		char *s; // TYPE_NAME and IDENTIFIER both use this
	} id;

	struct {
		int tok_tag;
		char *s;
	} str; // the same as id

#define CONST_VAL_TOK_INTEGER 1
#define CONST_VAL_TOK_FLOAT 2
	struct {
		int tok_tag;
		int flags;
		union {
			int ival;
			double fval;
		};
	} const_val;
};

enum {
#define DEFV(tok, v) tok = v,
#define DEF(tok) tok,
#define DEFM(tok) tok,
#include "lex/token.def"
#undef DEFV
#undef DEF
#undef DEFM
	TOK_TOTAL_NUM,
};

void token_destroy(union token token);
void token_dump(union token token);
void token_list_dump(struct dynarr *darr);
const char *token_tag_str(int tok_tag);
union token wrap_int_const_to_token(int val);
union token *token_shallow_dup(union token *inp);
union token *token_deep_dup(union token *inp);
union token wrap_to_simple_token(int tag);

#ifdef __cplusplus
}
#endif

#endif
