#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inc/parser.h>
#include <inc/dynarr.h>
#include <inc/util.h>

static struct external_declaration *parse_external_decl(struct parser *parser);
static struct init_declarator *parse_init_declarator_with_la(struct parser *parser, struct declarator *declarator);
static struct init_declarator_list *parse_init_declarator_list_with_la(struct parser *parser, struct declarator *declarator);
static struct declarator *parse_declarator(struct parser *parser);

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

static struct declaration_specifiers *parse_decl_specifiers(struct parser *parser) {
	union token tok;
	void *nd;
	struct dynarr *darr = dynarr_init();

	while (1) { // TODO handle more cases
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_INT) {
			nd = type_specifier_init(tok.tok_tag);
			token_destroy(tok);
		} else {
			lexer_put_back(parser->lexer, tok);
			break;
		}
		dynarr_add(darr, nd);
	}

	// the list should not be empty
	assert(dynarr_size(darr) > 0);
	return declaration_specifiers_init(darr);
}

// TODO parse trailing '[' or '('
// TODO parse '(' declarator ')'
static struct direct_declarator *parse_direct_declarator(struct parser *parser) {
	union token tok = expect(parser->lexer, TOK_IDENTIFIER);
	struct direct_declarator *dd = direct_declarator_init();
	dd->id = tok.id.s;

	tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_LPAREN) {
		expect(parser->lexer, TOK_RPAREN);
	} else {
		lexer_put_back(parser->lexer, tok);
	}
	return dd;
}

// TODO: handle pointer here
static struct declarator *parse_declarator(struct parser *parser) {
	struct direct_declarator *direct_declarator = parse_direct_declarator(parser);
	struct declarator *declarator = declarator_init();
	declarator->directDeclarator = direct_declarator;
	return declarator;
}

static struct initializer *parse_initializer(struct parser *parser) {
	panic("parse_initializer ni");
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
		return init_declarator_list_init(dynarr_init());
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
	struct declaration_specifiers *decl_specifiers = parse_decl_specifiers(parser);
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
	struct declaration_specifiers *decl_specifiers = parse_decl_specifiers(parser);
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
		init_declarator_list = init_declarator_list_init(dynarr_init());

		// set external decl
		external_decl->init_declarator_list = init_declarator_list;
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

