#include <inc/pp.h>

int push_want_newline(struct lexer *lexer, int newval) {
	int oldval = lexer->want_newline;
	lexer->want_newline = newval;
	return oldval;
}

void pop_want_newline(struct lexer *lexer, int oldval) {
	lexer->want_newline = oldval;
}
