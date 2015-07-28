#include <inc/cgc.h>
#include <inc/util.h>
#include <inc/token.h>

static const char *op_str_table[] = {
	[TOK_AMPERSAND] = "&",
	[TOK_ASSIGN] = "=",
	[TOK_ADD] = "+",
	[TOK_STAR] = "*",
	[TOK_SUB] = "-",
	[TOK_NE] = "!=",
	[TOK_EQ] = "==",
	[TOK_LE] = "<=",
	[TOK_LT] = "<",
	[TOK_INC] = "++",
	[TOK_ADD_ASSIGN] = "+=",
	[TOK_RSHIFT] = ">>",
	[TOK_LSHIFT] = "<<",
	[TOK_EXCLAMATION] = "!",
	[TOK_VERT_BAR] = "|",
	[TOK_DIV] = "/",
	[TOK_TOTAL_NUM] = NULL, // to make sure memory is allocated
};

const char *cgc_get_op_str(int tok_tag) {
	if (tok_tag < TOK_TOTAL_NUM && tok_tag >= 0) {
		const char *cstr = op_str_table[tok_tag];
		if (cstr != NULL) {
			return cstr;
		}
	}
	panic("not supported %s", token_tag_str(tok_tag));
}
