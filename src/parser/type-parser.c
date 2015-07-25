#include <inc/parser.h>
#include <inc/syntree.h>
#include <inc/util.h>

static struct struct_declaration_list *parse_struct_declaration_list(struct parser *parser) {
	panic("ni");
}

struct type_specifier *parse_struct_or_union_specifier(struct parser *parser) {
	union token struct_union_tok = lexer_next_token(parser->lexer);
	union token tok = lexer_next_token(parser->lexer);
	char *name = NULL;
	struct struct_declaration_list *struct_decl_list = NULL;

	assert(struct_union_tok.tok_tag == TOK_STRUCT || struct_union_tok.tok_tag == TOK_UNION);

	if (tok.tok_tag == TOK_IDENTIFIER) {
		name = tok.id.s;
		tok = lexer_next_token(parser->lexer);
	}

	if (name == NULL && tok.tok_tag != TOK_LBRACE) {
		panic("struct/union should be followed by a name or '{'");
	}

	if (tok.tok_tag == TOK_LBRACE) {
		struct_decl_list = parse_struct_declaration_list(parser);
	} else {
		lexer_put_back(parser->lexer, tok);
	}

	struct type_specifier *ret = type_specifier_init(struct_union_tok.tok_tag);
	ret->type_name = name;
	ret->struct_decl_list = struct_decl_list;
	return ret;
}
