#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inc/parser.h>
#include <inc/dynarr.h>
#include <inc/util.h>
#include <inc/dynarr.h>

#undef DEBUG
#define DEBUG 0

static struct external_declaration *parse_external_decl(struct parser *parser);
static struct init_declarator *parse_init_declarator_with_la(struct parser *parser, struct declarator *declarator);
static struct init_declarator_list *parse_init_declarator_list_with_la(struct parser *parser, struct declarator *declarator);
static struct declarator *parse_declarator(struct parser *parser);
static int initiate_type_qualifier(union token tok);
static int initiate_type_specifier(union token tok);

struct parser *parser_init(struct lexer *lexer) {
	struct parser *parser = malloc(sizeof(*parser));
	parser->lexer = lexer;
	return parser;
}

void parser_destroy(struct parser *parser) {
	free(parser);
}

struct syntree *parse(struct parser *parser) {
	struct translation_unit *tu = translation_unit_init();
	struct external_declaration *external_decl;
	union token tok;
	while ((tok = lexer_next_token(parser->lexer)).tok_tag != TOK_EOF) {
		lexer_put_back(parser->lexer, tok);
		external_decl = parse_external_decl(parser);
		dynarr_add(tu->external_decl_list, external_decl);
	}

	return syntree_init(tu);
}

int initiate_type_name(union token tok) {
	return initiate_type_specifier(tok) || initiate_type_qualifier(tok);
}

// TODO handle TYPE_NAME here
static int initiate_type_specifier(union token tok) {
	static int type_tag_list[] = {
		TOK_VOID, TOK_CHAR, TOK_SHORT, TOK_INT, TOK_LONG, TOK_FLOAT,
		TOK_DOUBLE, TOK_SIGNED, TOK_UNSIGNED, TOK_STRUCT, TOK_UNION,
		TOK_ENUM,
		TOK_UNDEF, // the last one
	};
	int i;
	for (i = 0; type_tag_list[i] != TOK_UNDEF; i++) {
		if (type_tag_list[i] == tok.tok_tag) {
			return 1;
		}
	}
	return 0;
}

static int initiate_storage_class_specifier(union token tok) {
	return tok.tok_tag == TOK_TYPEDEF
		|| tok.tok_tag == TOK_EXTERN
		|| tok.tok_tag == TOK_STATIC
		|| tok.tok_tag == TOK_AUTO
		|| tok.tok_tag == TOK_REGISTER;
}

/* TODO handle TYPE_NAME
 * We already know that a type_specifier is following
 */
static struct type_specifier *parse_type_specifier(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_STRUCT || tok.tok_tag == TOK_UNION || tok.tok_tag == TOK_ENUM) {
		panic("struct, union, enum not supported yet");
	} else {
		return type_specifier_init(tok.tok_tag);
	}
}

static struct declaration_specifiers *parse_declaration_specifiers(struct parser *parser) {
	union token tok;
	void *nd;
	struct dynarr *darr = dynarr_init();

	while (1) { 
		tok = lexer_next_token(parser->lexer);
		if (initiate_type_specifier(tok)) {
			lexer_put_back(parser->lexer, tok);
			nd = parse_type_specifier(parser);
		} else if (initiate_type_qualifier(tok)) {
			nd = type_qualifier_init(tok.tok_tag);
		} else if (initiate_storage_class_specifier(tok)) {
			nd = storage_class_specifier_init(tok.tok_tag);
		} else {
			lexer_put_back(parser->lexer, tok);
			break;
		}
		dynarr_add(darr, nd);
	}

#if DEBUG
	if (dynarr_size(darr) == 0) {
		lexer_dump_remaining(parser->lexer); // TODO
	}
#endif

	assert(dynarr_size(darr) > 0);
	return declaration_specifiers_init(darr);
}

static int initiate_declarator(union token tok) {
	return tok.tok_tag == TOK_STAR || tok.tok_tag == TOK_IDENTIFIER || tok.tok_tag == TOK_LPAREN;
}

// TODO support abstract_declarator
static struct parameter_declaration *parse_parameter_declaration(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_declaration_specifiers(parser);

	union token tok = lexer_next_token(parser->lexer);
	struct declarator *declarator = NULL;
	if (initiate_declarator(tok)) {
		lexer_put_back(parser->lexer, tok);
		declarator = parse_declarator(parser);
	} else {
		lexer_put_back(parser->lexer, tok);
	}
	return parameter_declaration_init(decl_specifiers, declarator);
}

static struct parameter_type_list *parse_parameter_type_list(struct parser *parser) {
	struct parameter_type_list *param_type_list = parameter_type_list_init();
	struct parameter_declaration *decl = parse_parameter_declaration(parser);
	dynarr_add(param_type_list->param_decl_list, decl);
	union token tok;

	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_COMMA) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_ELLIPSIS) {
			param_type_list->has_ellipsis = 1;
			break;
		}
		lexer_put_back(parser->lexer, tok);
		decl = parse_parameter_declaration(parser);
		dynarr_add(param_type_list->param_decl_list, decl);
	}
	return param_type_list;
}

static struct direct_declarator *parse_direct_declarator(struct parser *parser) {
	struct direct_declarator *dd = direct_declarator_init();
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_IDENTIFIER) {
		dd->id = tok.id.s;	
	} else if (tok.tok_tag == TOK_LPAREN) {
		dd->declarator = parse_declarator(parser);
		expect(parser->lexer, TOK_RPAREN);
	} else {
		panic("unexpected token %s\n", token_tag_str(tok.tok_tag));
	}
	
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_LBRACKET) {
			tok = lexer_next_token(parser->lexer);
			struct direct_declarator_suffix *suff = mallocz(sizeof(*suff));
			if (tok.tok_tag == TOK_RBRACKET) {
				suff->empty_bracket = 1;
			} else {
				lexer_put_back(parser->lexer, tok);
				struct constant_expression *expr = parse_constant_expression(parser);
				expect(parser->lexer, TOK_RBRACKET);
				suff->const_expr = expr;
			}
			dynarr_add(dd->suff_list, suff);
		} else if (tok.tok_tag == TOK_LPAREN) {
			tok = lexer_next_token(parser->lexer);
			struct direct_declarator_suffix *suff = mallocz(sizeof(*suff));
			if (tok.tok_tag == TOK_RPAREN) {
				suff->empty_paren = 1;
			} else {
				lexer_put_back(parser->lexer, tok);
				struct parameter_type_list *param_type_list = parse_parameter_type_list(parser);
				expect(parser->lexer, TOK_RPAREN);
				suff->param_type_list = param_type_list;
			}
			dynarr_add(dd->suff_list, suff);
		} else {
			lexer_put_back(parser->lexer, tok);	
			break;
		}
	}
	return dd;
}

static int initiate_type_qualifier(union token tok) {
	return tok.tok_tag == TOK_CONST || tok.tok_tag == TOK_VOLATILE;
}

static struct dynarr *parse_pointer(struct parser *parser) {
	union token tok;
	struct type_qualifier_list *qual_list = NULL;
	struct dynarr *ptr_list = dynarr_init();
	while (1) {
		// get '*'
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_STAR) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		// get the following type qualifier list	
		qual_list = type_qualifier_list_init();
		while (1) {
			tok = lexer_next_token(parser->lexer);
			if (!initiate_type_qualifier(tok)) {
				lexer_put_back(parser->lexer, tok);
				break;
			}

			dynarr_add(qual_list->darr, type_qualifier_init(tok.tok_tag));
		}
		dynarr_add(ptr_list, qual_list);
	}
	return ptr_list;
}

static struct declarator *parse_declarator(struct parser *parser) {
	struct dynarr *ptr_list = parse_pointer(parser);
	struct direct_declarator *direct_declarator = parse_direct_declarator(parser);
	struct declarator *declarator = declarator_init();
	declarator->ptr_list = ptr_list;
	declarator->direct_declarator = direct_declarator;
	return declarator;
}

static struct initializer *parse_initializer(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	struct initializer *initializer = initializer_init();
	if (tok.tok_tag == TOK_LBRACE) {
		panic("struct intializer is not supported yet");
	}
	lexer_put_back(parser->lexer, tok);
	initializer->expr = parse_assignment_expression(parser);
	return initializer;
}

static struct init_declarator *parse_init_declarator(struct parser *parser) {
	struct declarator *declarator = parse_declarator(parser);	
	return parse_init_declarator_with_la(parser, declarator);	
}

static struct init_declarator *parse_init_declarator_with_la(struct parser *parser, struct declarator *declarator) {
	union token tok = lexer_next_token(parser->lexer);
	struct initializer *initializer = NULL;
	if (tok.tok_tag == TOK_ASSIGN) {
		initializer = parse_initializer(parser);
	} else {
		lexer_put_back(parser->lexer, tok); // put back the look ahead token
	}
	return init_declarator_init(declarator, initializer);
}

// parse init declarator util meeting a ';'
static struct init_declarator_list *parse_init_declarator_list(struct parser *parser) {
	// need check for empty init_declarator_list here
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_SEMICOLON) {
		return NULL;
	}
	lexer_put_back(parser->lexer, tok);
	return parse_init_declarator_list_with_la(parser, 
		parse_declarator(parser));
}

static struct init_declarator_list *parse_init_declarator_list_with_la(struct parser *parser, struct declarator *declarator) {
	assert(declarator != NULL); 
	struct dynarr *darr = dynarr_init();
	struct init_declarator *init_declarator = parse_init_declarator_with_la(parser, declarator);;
	union token tok;

	while (1) {
		dynarr_add(darr, init_declarator);
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_COMMA) {
			// go on for next init_declarator
		} else if (tok.tok_tag == TOK_SEMICOLON) {
			break;
		} else {
			panic("expect ',' or ';'");
		}
		init_declarator = parse_init_declarator(parser);
	}
	return init_declarator_list_init(darr);
}

struct declaration *parse_declaration(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_declaration_specifiers(parser);
	struct init_declarator_list *init_declarator_list = parse_init_declarator_list(parser);
	return declaration_init(decl_specifiers, init_declarator_list);
}

/*
 * Checks if the token initiates a declaration (or statement). Used by parse_compound_statement
 */
int initiate_declaration(union token tok) {
	return tok.tok_tag == TOK_INT; // TODO: refine this
}

// assume no EOF found; 
static struct external_declaration *parse_external_decl(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_declaration_specifiers(parser);
	struct external_declaration *external_decl = external_declaration_init(decl_specifiers);
	struct declarator *declarator = NULL;

	// compound statement case
	struct compound_statement *compound_stmt = NULL;
	// declaration case
	struct init_declarator_list *init_declarator_list = NULL;

	// check for empty init_declarator_list case, similar to what we do in
	// parse_init_declarator_list
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_SEMICOLON) {
	} else {
		lexer_put_back(parser->lexer, tok);
		declarator = parse_declarator(parser);

		tok = lexer_next_token(parser->lexer);	
		if (tok.tok_tag == TOK_LBRACE) {
			lexer_put_back(parser->lexer, tok);
			compound_stmt = parse_compound_statement(parser);
		
			// set external decl
			external_decl->func_def_declarator = declarator;
			external_decl->compound_stmt = compound_stmt;
		} else {
			lexer_put_back(parser->lexer, tok);
			init_declarator_list = parse_init_declarator_list_with_la(parser, declarator);

			// set external decl
			external_decl->init_declarator_list = init_declarator_list;
		}
	}

	return external_decl;
}

