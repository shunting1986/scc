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
