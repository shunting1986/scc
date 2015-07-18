#ifndef _INC_TOKEN_H
#define _INC_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

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
#include "lex/token.def"
#undef DEFV
#undef DEF
	TOK_TOTAL_NUM,
};

void token_destroy(union token token);
void token_dump(union token token);
const char *token_tag_str(int tok_tag);
union token wrap_int_const_to_token(int val);

#ifdef __cplusplus
}
#endif

#endif
