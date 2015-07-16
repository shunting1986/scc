#ifndef _INC_SYMTAB_H
#define _INC_SYMTAB_H

#include <inc/htab.h>
#include <inc/type.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYMBOL_MAX_LEN 128

struct symtab {
	struct symtab *enclosing;
	struct hashtab *htab;
};

enum {
	SYMBOL_GLOBAL_VAR,
	SYMBOL_LOCAL_VAR,
	SYMBOL_PARAM,
};

struct symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
};

struct global_var_symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
};

struct local_var_symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
	int var_ind;
};

struct param_symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
	int param_ind;
};

struct symtab *symtab_init(struct symtab *enclosing);
void symtab_destroy(struct symtab *stab);
struct symbol *symtab_lookup_norec(struct symtab *stab, const char *id);
struct symbol *symtab_new_param(char *name, int ind);
struct symbol *symtab_new_local_var(char *name, int ind, struct type *ctype);
struct symbol *symtab_new_global_var(char *name, struct type *ctype);
void symtab_add(struct symtab *stab, struct symbol *sym);
struct symbol *symtab_lookup(struct symtab *stab, const char *id);
void symtab_iter(struct symtab *symtab, void *ctx, htab_iter_fn_type *func);

void symbol_destroy(void *_sym);

#ifdef __cplusplus
}
#endif

#endif
