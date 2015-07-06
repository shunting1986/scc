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
	panic("ni");
}

void symtab_add(struct symtab *stab, struct symbol *sym) {
	panic("ni");
}

struct symbol *symtab_new_param(char *name, int ind) {
	panic("ni");
}

struct symbol *symtab_new_local_var(char *name, int ind) {
	panic("ni");
}

struct symbol *symtab_new_global_var(char *name) {
	panic("ni");
}
