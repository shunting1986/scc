#include <string.h>

#include <inc/keyword.h>
#include <inc/lexer.h>
#include <inc/util.h>

// XXX: hash table is better here
// XXX: support all keywords
int check_keyword_token(char *s) {
	if (strcmp(s, "int") == 0) {
		return TOK_INT;
	} else if (strcmp(s, "return") == 0) {
		return TOK_RETURN;
	} else if (strcmp(s, "const") == 0) {
		return TOK_CONST;
	} else {
		return TOK_UNDEF;
	}
}

static const char *keywork_str_list[] = {
	[TOK_CONST] = "const",
	[TOK_TOTAL_NUM] = NULL,
};

const char *keyword_str(int tok_tag) {
	if (tok_tag >= 0 && tok_tag < TOK_TOTAL_NUM) {
		const char *cstr = keywork_str_list[tok_tag];
		if (cstr != NULL) {
			return cstr;
		}
	}
	panic("not support %s", token_tag_str(tok_tag));
}
