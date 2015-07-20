#include <inc/pp.h>
#include <inc/util.h>

int push_want_newline(struct lexer *lexer, int newval) {
	int oldval = lexer->want_newline;
	lexer->want_newline = newval;
	return oldval;
}

void pop_want_newline(struct lexer *lexer, int oldval) {
	lexer->want_newline = oldval;
}

void pp_push_if_item(struct lexer *lexer, int item, int flag) {
	dynarr_add(lexer->if_stack, (void *) (long) (item | flag));
}

int pp_in_skip_mode(struct lexer *lexer) {
	int ret = false;
	if (dynarr_size(lexer->if_stack) > 0)	{
		int top_item = (int) (long) dynarr_last(lexer->if_stack);
		ret = (top_item & IF_FLAG_ALWAYS_SKIP) || IF_ITEM_VALUE(top_item) == IF_FALSE;
	}
	return ret;
}
