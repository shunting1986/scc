#include <inc/parser.h>
#include <inc/syntree.h>
#include <inc/util.h>

struct struct_declarator *parse_struct_declarator(struct parser *parser) {
	struct declarator *declarator = NULL;
	struct constant_expression *const_expr = NULL;
	union token tok = lexer_next_token(parser->lexer);

	if (tok.tok_tag != TOK_COLON) {
		lexer_put_back(parser->lexer, tok);
		declarator = parse_declarator(parser);
		tok = lexer_next_token(parser->lexer);
	}
	
	if (tok.tok_tag == TOK_COLON) {
		const_expr = parse_constant_expression(parser);
	} else {
		lexer_put_back(parser->lexer, tok);
	}

	return struct_declarator_init(declarator, const_expr);
}

static struct struct_declaration *parse_struct_declaration(struct parser *parser) {
	struct specifier_qualifier_list *sqlist = parse_specifier_qualifier_list(parser);
	struct dynarr *declarator_list = dynarr_init();
	union token tok;

	while (true) {
		dynarr_add(declarator_list, parse_struct_declarator(parser));
		tok = lexer_next_token(parser->lexer);

		if (tok.tok_tag == TOK_SEMICOLON) {
			break;
		}
		assume(tok, TOK_COMMA);
	}
	return struct_declaration_init(sqlist, declarator_list);
}

static struct struct_declaration_list *parse_struct_declaration_list(struct parser *parser) {
	union token tok;
	struct dynarr *darr = dynarr_init();
	while (true) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_RBRACE) {
			break;
		}

		lexer_put_back(parser->lexer, tok);
		dynarr_add(darr, parse_struct_declaration(parser));
	}
	return struct_declaration_list_init(darr);
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
