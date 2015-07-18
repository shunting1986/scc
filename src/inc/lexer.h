#ifndef _INC_LEXER_H
#define _INC_LEXER_H

#include <inc/file_reader.h>
#include <inc/token.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOOKAHEAD_NUM 2 // then maximum number of look aheads

struct typedef_tab;

struct lexer {
	struct file_reader *cstream;

	// it's more reasonable to use a stack rather than a queue
	union token putback_stk[LOOKAHEAD_NUM];
	int nputback;

	struct typedef_tab *typedef_tab;
};

struct lexer *lexer_init(struct file_reader *cstream);
union token lexer_next_token(struct lexer *lexer);
union token expect(struct lexer *lexer, int tok_tag);
void lexer_put_back(struct lexer *lexer, union token token);
void lexer_destroy(struct lexer *lexer);
void lexer_dump_remaining(struct lexer *lexer);

/* typedef.c */
int lexer_is_typedef(struct lexer *lexer, const char *s);
struct typedef_tab *typedef_tab_init(struct typedef_tab *enclosing);
void lexer_push_typedef_tab(struct lexer *lexer);
void lexer_pop_typedef_tab(struct lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif
