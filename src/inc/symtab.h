#ifndef _INC_SYMTAB_H
#define _INC_SYMTAB_H

#ifdef __cplusplus
extern "C" {
#endif

struct symtab {
	struct symtab *enclosing;
	struct hashtab *htab;
};

struct symbol {
	int type;
	char *name;
};

struct global_var_symbol {
	int type;
	char *name;
};

struct local_var_symbol {
	int type;
	char *name;
	int var_ind;
};

struct param_symbol {
	int type;
	char *name;
	int param_ind;
};

struct symtab *symtab_init(struct symtab *enclosing);
void symtab_destroy(struct symtab *stab);
struct symbol *symtab_lookup_norec(struct symtab *stab, const char *id);
struct symbol *symtab_new_param(char *name, int ind);
struct symbol *symtab_new_local_var(char *name, int ind);
struct symbol *symtab_new_global_var(char *name);
void symtab_add(struct symtab *stab, struct symbol *sym);

#ifdef __cplusplus
}
#endif

#endif
