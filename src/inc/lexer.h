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
	int typedef_disabled;

	int in_pp_context; // indicate if we are in preprocessor context

	int want_newline;

	struct hashtab *macro_tab;
	struct dynarr *if_stack;

	struct dynarr *expanded_macro; // set the item to NULL when any elem is used
		// the macro is fully expanded
	int expanded_macro_pos;
};

struct lexer *lexer_init(struct file_reader *cstream);
union token lexer_next_token(struct lexer *lexer);
union token expect(struct lexer *lexer, int tok_tag);
void assume(union token tok, int tok_tag);
void lexer_put_back(struct lexer *lexer, union token token);
void lexer_destroy(struct lexer *lexer);
void lexer_dump_remaining(struct lexer *lexer);
char *parse_string_literal(struct lexer *lexer, int term_tag);
void lexer_discard_line(struct lexer *lexer);
void normalize_expanded_token_list(struct lexer *lexer);

/* typedef.c */
int lexer_is_typedef(struct lexer *lexer, const char *s);
struct typedef_tab *typedef_tab_init(struct typedef_tab *enclosing);
void lexer_push_typedef_tab(struct lexer *lexer);
void lexer_pop_typedef_tab(struct lexer *lexer);
void lexer_register_typedef(struct lexer *lexer, char *id);

#ifdef __cplusplus
}
#endif

#endif
