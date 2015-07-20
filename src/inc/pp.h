#ifndef _INC_PP_H
#define _INC_PP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/lexer.h>


/* This flag is set if the block is enclosed within a larger skip context or 
 * if this block is part of an if-elsif sequence whose true part already happen
 */
#define IF_FLAG_ALWAYS_SKIP 0x40000000

#define IF_FLAG_MASK 0xFF000000

#define IF_ITEM_VALUE(v) ((v) & ~IF_FLAG_MASK)

enum {
	IF_TRUE = 0,
	IF_FALSE,
};

// pp.c
void pp_entry(struct lexer *lexer);

// keyword.c
int check_pp_keyword(char *s);

// pp-util.c
int push_want_newline(struct lexer *lexer, int newval);
void pop_want_newline(struct lexer *lexer, int oldval);
void pp_push_if_item(struct lexer *lexer, int item, int flag);
int pp_in_skip_mode(struct lexer *lexer);

// pp-include.c
void pp_include(struct lexer *lexer);

// macro-symtab.c
int macro_defined(struct lexer *lexer, const char *s);

// macro.c
void macro_destroy(void *_macro);

// pp-expr.c
int pp_expr(struct lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif
