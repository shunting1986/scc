#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inc/parser.h>
#include <inc/dynarr.h>
#include <inc/util.h>

static struct external_decl_node *parse_external_decl(struct parser *parser);

struct parser *parser_init(struct lexer *lexer) {
	struct parser *parser = malloc(sizeof(*parser));
	parser->lexer = lexer;
	return parser;
}

void parser_destroy(struct parser *parser) {
	free(parser);
}

struct syntree *parse(struct parser *parser) {
	union token tok;
	while ((tok = lexer_next_token(parser->lexer)).tok_tag != TOK_EOF) {
		// lexer put back, should not destroy tok
		lexer_put_back(parser->lexer, tok);
		struct external_decl_node *external_decl = parse_external_decl(parser);
	}
	panic("parse ni"); // TODO
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
	struct dynarr *darr = dynarr_init();
	struct init_declarator *init_declarator;
	union token tok;
	while (1) {
		init_declarator = parse_init_declarator(parser);
		dynarr_add(darr, init_declarator);
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_COMMA) {
			// go on for next init_declarator
		} else 
		if (tok.tok_tag == TOK_SEMICOLON) {
			break;
		} else {
			expect(parser->lexer, TOK_COMMA);
		}
	}
	return init_declarator_list_init(darr);
}

static struct declaration *parse_declaration(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_decl_specifiers(parser);
	struct init_declarator_list *init_declarator_list = parse_init_declarator_list(parser);
	return declaration_init(decl_specifiers, init_declarator_list);
}

/*
 * Checks if the token initiates a declaration (or statement). Used by parse_compound_statement
 */
static int initiateDeclaration(union token tok) {
	return tok.tok_tag == TOK_INT; // TODO: refine this
}

static struct compound_statement *parse_compound_statement(struct parser *parser) {
	expect(parser->lexer, TOK_LBRACE);

	// look one token ahead to determing if this is a declaration or statement or empty block
	union token tok = lexer_next_token(parser->lexer);
	struct dynarr *declList = dynarr_init();
	struct dynarr *stmtList = dynarr_init();
	while (tok.tok_tag != TOK_RBRACE) {
		if (initiateDeclaration(tok)) {
			if (dynarr_size(stmtList) > 0) {
				panic("encounter declaration after statement");
			}
			lexer_put_back(parser->lexer, tok);
			dynarr_add(declList, parse_declaration(parser));
		} else { // initiate a statement
			lexer_put_back(parser->lexer, tok);
			dynarr_add(stmtList, parse_statement(parser));
		}
		tok = lexer_next_token(parser->lexer);
	}
	return compound_statement_init(declList, stmtList);
}

// assume no EOF found; 
static struct external_decl_node *parse_external_decl(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_decl_specifiers(parser);
	struct declarator *declarator = parse_declarator(parser);

	// TODO: parse general declaration here, right now this function only focus on function definition
	// TODO: can reuse the code in parse_declaration

	struct compound_statement *compoundStmt = parse_compound_statement(parser);

	panic("parse_external_decl ni"); 
}

