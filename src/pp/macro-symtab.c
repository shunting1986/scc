#include <inc/pp.h>
#include <inc/util.h>
#include <inc/htab.h>

int macro_defined(struct lexer *lexer, const char *s) {
	return htab_query(lexer->macro_tab, s) != NULL;
}
