#include <inc/parser.h>
#include <inc/syntree.h>
#include <inc/util.h>

static struct enumerator *parse_enumerator(struct parser *parser) {
	union token id_tok = expect(parser->lexer, TOK_IDENTIFIER);
	union token tok = lexer_next_token(parser->lexer);
	struct constant_expression *expr = NULL;

	if (tok.tok_tag == TOK_ASSIGN) {
		expr = parse_constant_expression(parser);
	} else {
		lexer_put_back(parser->lexer, tok);
	}
	return enumerator_init(id_tok.id.s, expr);
}

static struct enumerator_list *parse_enumerator_list(struct parser *parser) {
	union token tok;
	struct dynarr *darr = dynarr_init();

	tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag != TOK_RBRACE) {
		lexer_put_back(parser->lexer, tok);
		while (true) {
			dynarr_add(darr, parse_enumerator(parser));
			tok = lexer_next_token(parser->lexer);
			if (tok.tok_tag == TOK_RBRACE) {
				break;
			}
			assume(tok, TOK_COMMA);

			// check for TOK_RBRACE again since enum definition can have an extra last comma
			tok = lexer_next_token(parser->lexer);
			if (tok.tok_tag == TOK_RBRACE) {
				break;
			}
			lexer_put_back(parser->lexer, tok);
		}
	}

	return enumerator_list_init(darr);
}

struct type_specifier *parse_enum_specifier(struct parser *parser) {
	char *name = NULL;
	union token tok = lexer_next_token(parser->lexer);
	struct enumerator_list *enumerator_list = NULL;

	if (tok.tok_tag == TOK_IDENTIFIER) {
		name = tok.id.s;
		tok = lexer_next_token(parser->lexer);
	}

	if (name == NULL && tok.tok_tag != TOK_LBRACE) {
		panic("enum should be followed by a name or '{'");
	}

	if (tok.tok_tag == TOK_LBRACE) {
		enumerator_list = parse_enumerator_list(parser);
	} else {
		lexer_put_back(parser->lexer, tok);
	}

	struct type_specifier *ret = type_specifier_init(TOK_ENUM);
	ret->type_name = name;
	ret->enumerator_list = enumerator_list;
	return ret;
}

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
		file_reader_dump_remaining(parser->lexer->cstream);
		token_dump(tok);
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
