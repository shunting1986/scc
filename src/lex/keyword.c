#include <string.h>

#include <inc/keyword.h>
#include <inc/lexer.h>

// XXX: hash table is better here
// XXX: support all keywords
int check_keyword_token(char *s) {
	if (strcmp(s, "int") == 0) {
		return TOK_INT;
	} else {
		return TOK_UNDEF;
	}
}
