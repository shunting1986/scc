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
	TOK_INT,

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
