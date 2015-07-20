#include <inc/pp.h>
#include <inc/util.h>
#include <inc/htab.h>

int macro_defined(struct lexer *lexer, const char *s) {
	return htab_query(lexer->macro_tab, s) != NULL;
}

void define_macro(struct lexer *lexer, const char *name, struct macro *macro) {
	// check for token redefine
	// TODO the two htab access actually can be merged to a single one
	if (htab_query(lexer->macro_tab, name) != NULL) {
		panic("redefine %s", name);
	}
	htab_insert(lexer->macro_tab, name, macro);
}

void undef_macro(struct lexer *lexer, const char *name) {
	htab_delete(lexer->macro_tab, name);
}
