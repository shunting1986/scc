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
		char *s;
	} str; // the same as id

	struct {
		int tok_tag;
		union {
			int ival;
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

#ifdef __cplusplus
}
#endif

#endif
