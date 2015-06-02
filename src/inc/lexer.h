#ifndef _INC_LEXER_H
#define _INC_LEXER_H

#include <inc/file_reader.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lexer {
	struct file_reader *cstream;
};

enum {
	TOK_INT,

	TOK_EOF,
};

union token {
	int token_tag;
};

struct lexer *lexer_init(struct file_reader *cstream);
union token lexer_next_token(struct lexer *lexer);
void lexer_destroy(struct lexer *lexer);
void token_destroy(union token token);
void token_dump(union token token);

#ifdef __cplusplus
}
#endif

#endif
