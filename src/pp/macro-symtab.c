#include <inc/pp.h>
#include <inc/util.h>
#include <inc/htab.h>

struct macro *query_macro_tab(struct lexer *lexer, const char *s) {
	return htab_query(lexer->macro_tab, s);
}

int macro_defined(struct lexer *lexer, const char *s) {
	return htab_query(lexer->macro_tab, s) != NULL;
}

void define_macro(struct lexer *lexer, const char *name, struct macro *macro) {
	// TODO the two htab access actually can be merged to a single one

	/*
	 * GCC rule for macro redefinition is:
	 * if the redefinition is the same as the previous one, then everything is fine;
	 * otherwise, a warning (not error) shows up.
	 *
	 * Our rule can be simpler: just redefine the macro and always print a message for redefinition
	 *
	 * NOTE:
   *   GCC treat the following two as different ones, so an warning (not error) will pop up 
	 *   #define ZZ(c, b) c + b
	 *   #define ZZ(a, b) a + b
	 */
	struct macro *old_macro;
	if ((old_macro = htab_query(lexer->macro_tab, name)) != NULL) {
		fprintf(stderr, "\033[31mredefine macro: %s\033[0m\n", name);
		htab_delete(lexer->macro_tab, name); // delete the old one
	}
	htab_insert(lexer->macro_tab, name, macro);
}

void undef_macro(struct lexer *lexer, const char *name) {
	htab_delete(lexer->macro_tab, name);
}
