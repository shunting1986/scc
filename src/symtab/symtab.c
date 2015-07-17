#include <inc/symtab.h>
#include <inc/util.h>

struct symtab *symtab_init(struct symtab *enclosing) {
	struct symtab *tab = (struct symtab *) mallocz(sizeof(*tab));
	tab->enclosing = enclosing;
	tab->htab = htab_init();
	tab->htab->val_free_fn = symbol_destroy;
	return tab;
}

/*
 * XXX: the current implementation is iterate thru the hash table. If this becomes
 * a bottleneck, we can optimize by maintaining a parallel list. This change can 
 * be transparent to the upper level code.
 *
 * This method does not recursively iterate thru the enclosing symbol table
 */
void symtab_iter(struct symtab *symtab, void *ctx, htab_iter_fn_type *func) {
	htab_iter(symtab->htab, ctx, func);
}

void symtab_destroy(struct symtab *stab) {
	htab_destroy(stab->htab);
	free(stab);
}

/*
 * Only check the current level (no enclosing level)
 */
struct symbol *symtab_lookup_norec(struct symtab *stab, const char *id) {
	return htab_query(stab->htab, id);
}

struct symbol *symtab_lookup(struct symtab *stab, const char *id) {
	void *ret;
	while (stab) {
		if ((ret = htab_query(stab->htab, id)) != NULL) {
			return ret;
		}
		stab = stab->enclosing;
	}
	return NULL;
}

void symtab_add(struct symtab *stab, struct symbol *sym) {
	htab_insert(stab->htab, sym->name, sym);
}

struct symbol *symtab_new_param(char *name, int ind, struct type *ctype) {
	struct param_symbol *sym = mallocz(sizeof(*sym));
	sym->type = SYMBOL_PARAM;
	sym->ctype = ctype;
	strncpy(sym->name, name, SYMBOL_MAX_LEN - 1);
	sym->param_ind = ind;
	return (struct symbol *) sym;
}

struct symbol *symtab_new_local_var(char *name, int ind, struct type *ctype) {
	struct local_var_symbol *sym = mallocz(sizeof(*sym));
	sym->type = SYMBOL_LOCAL_VAR;
	sym->ctype = ctype;
	strncpy(sym->name, name, SYMBOL_MAX_LEN - 1);
	sym->var_ind = ind;
	return (struct symbol *) sym;
}

struct symbol *symtab_new_global_var(char *name, struct type *ctype) {
	struct global_var_symbol *sym = mallocz(sizeof(*sym));
	sym->type = SYMBOL_GLOBAL_VAR;
	sym->ctype = ctype;
	strncpy(sym->name, name, SYMBOL_MAX_LEN - 1);
	return (struct symbol *) sym;
}

void symbol_destroy(void *_sym) {
	struct symbol *sym = _sym;
	if (sym->ctype) {
		type_destroy(sym->ctype);
	}
	free(sym);
}

