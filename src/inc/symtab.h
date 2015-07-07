#ifndef _INC_SYMTAB_H
#define _INC_SYMTAB_H

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
};

struct global_var_symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
};

struct local_var_symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
	int var_ind;
};

struct param_symbol {
	int type;
	char name[SYMBOL_MAX_LEN];
	int param_ind;
};

struct symtab *symtab_init(struct symtab *enclosing);
void symtab_destroy(struct symtab *stab);
struct symbol *symtab_lookup_norec(struct symtab *stab, const char *id);
struct symbol *symtab_new_param(char *name, int ind);
struct symbol *symtab_new_local_var(char *name, int ind);
struct symbol *symtab_new_global_var(char *name);
void symtab_add(struct symtab *stab, struct symbol *sym);
struct symbol *symtab_lookup(struct symtab *stab, const char *id);

#ifdef __cplusplus
}
#endif

#endif
