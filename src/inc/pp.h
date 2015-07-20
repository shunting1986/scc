#ifndef _INC_PP_H
#define _INC_PP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/lexer.h>

// pp.c
void pp_entry(struct lexer *lexer);

// keyword.c
int check_pp_keyword(char *s);

// pp-util.c
int push_want_newline(struct lexer *lexer, int newval);
void pop_want_newline(struct lexer *lexer, int oldval);

// macro-symtab.c
int macro_defined(struct lexer *lexer, const char *s);

// macro.c
void macro_destroy(void *_macro);

#ifdef __cplusplus
}
#endif

#endif
