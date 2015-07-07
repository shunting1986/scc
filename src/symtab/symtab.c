#include <inc/symtab.h>
#include <inc/util.h>
#include <inc/htab.h>

struct symtab *symtab_init(struct symtab *enclosing) {
	struct symtab *tab = (struct symtab *) mallocz(sizeof(*tab));
	tab->enclosing = enclosing;
	tab->htab = htab_init();
	return tab;
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

struct symbol *symtab_new_param(char *name, int ind) {
	panic("ni");
}

struct symbol *symtab_new_local_var(char *name, int ind) {
	struct local_var_symbol *sym = mallocz(sizeof(*sym));
	sym->type = SYMBOL_LOCAL_VAR;
	strncpy(sym->name, name, SYMBOL_MAX_LEN - 1);
	sym->var_ind = ind;
	return (struct symbol *) sym;
}

struct symbol *symtab_new_global_var(char *name) {
	panic("ni");
}
