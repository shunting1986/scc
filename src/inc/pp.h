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

#ifdef __cplusplus
}
#endif

#endif
