#include <inc/lexer.h>
#include <inc/util.h>

struct typedef_tab {
	struct typedef_tab *enclosing;
	struct hashtab *htab;
};

void lexer_register_typedef(struct lexer *lexer, char *id) {
	panic("ni");
}

void lexer_push_typedef_tab(struct lexer *lexer) {
	panic("ni");
}

void lexer_pop_typedef_tab(struct lexer *lexer) {
	panic("ni");
}

struct typedef_tab *typedef_tab_init(struct typedef_tab *enclosing) {
	panic("ni");
}

int lexer_is_typedef(struct lexer *lexer, const char *s) {
	panic("ni");
}


