#include <inc/util.h>
#include <inc/pp.h>
#include <inc/lexer.h>

void pp_entry(struct lexer *lexer) {
	int old_in_pp_context = lexer->in_pp_context;
	lexer->in_pp_context = 1;

	union token tok = lexer_next_token(lexer);
	
	panic("ni %s", token_tag_str(tok.tok_tag));

// out:
	lexer->in_pp_context = old_in_pp_context;
}
