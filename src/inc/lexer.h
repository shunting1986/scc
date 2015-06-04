#ifndef _INC_LEXER_H
#define _INC_LEXER_H

#include <inc/file_reader.h>
#include <inc/token.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lexer {
	struct file_reader *cstream;
	union token put_back;
};

struct lexer *lexer_init(struct file_reader *cstream);
union token lexer_next_token(struct lexer *lexer);
void lexer_put_back(struct lexer *lexer, union token token);
void lexer_destroy(struct lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif
