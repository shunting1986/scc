#ifndef _INC_PP_H
#define _INC_PP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/lexer.h>
#include <inc/util.h>

struct macro;

// pp.c
void pp_entry(struct lexer *lexer);

// keyword.c
int check_pp_keyword(char *s);

// pp-util.c
int push_want_newline(struct lexer *lexer, int newval);
void pop_want_newline(struct lexer *lexer, int oldval);
int push_want_quotation(struct lexer *lexer, int newval);
void pop_want_quotation(struct lexer *lexer, int oldval);

// pp-include.c
void pp_include(struct lexer *lexer);

// macro-symtab.c
int macro_defined(struct lexer *lexer, const char *s);
void define_macro(struct lexer *lexer, const char *name, struct macro *macro);
void undef_macro(struct lexer *lexer, const char *name);
struct macro *query_macro_tab(struct lexer *lexer, const char *s);

// macro.c
enum {
	MACRO_OBJ,
	MACRO_FUNC,
};

struct macro {
	int type;
	struct dynarr *toklist;
	union {
		struct dynarr *paramlist; // for func macro
	};
};
struct macro *obj_macro_init(struct dynarr *toklist);
struct macro *func_macro_init(struct dynarr *paramlist, struct dynarr *toklist);
void macro_destroy(void *_macro);
void macro_dump(const char *name, struct macro *macro);

// pp-expr.c
int pp_expr(struct lexer *lexer);

// pp-define.c
void pp_define(struct lexer *lexer);
void pp_undef(struct lexer *lexer);

// macro-expand.c
bool try_expand_macro(struct lexer *lexer, const char *name);

#ifdef __cplusplus
}
#endif

#endif
