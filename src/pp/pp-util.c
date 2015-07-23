#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>

// TODO use the macro
int push_want_quotation(struct lexer *lexer, int newval) {
	int oldval = lexer->want_quotation;
	lexer->want_quotation = newval;
	return oldval;
}

// TODO use the macro
void pop_want_quotation(struct lexer *lexer, int oldval) {
	lexer->want_quotation = oldval;
}

// TODO use the macro
int push_want_newline(struct lexer *lexer, int newval) {
	int oldval = lexer->want_newline;
	lexer->want_newline = newval;
	return oldval;
}

// TODO use the macro
void pop_want_newline(struct lexer *lexer, int oldval) {
	lexer->want_newline = oldval;
}


