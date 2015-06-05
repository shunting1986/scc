#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inc/parser.h>
#include <inc/dynarr.h>

struct parser {
	struct lexer *lexer;
};

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
	return dd;
}

// TODO: handle pointer here
static struct declarator *parse_declarator(struct parser *parser) {
	struct direct_declarator *direct_declarator = parse_direct_declarator(parser);
	struct declarator *declarator = declarator_init();
	declarator->directDeclarator = direct_declarator;
	return declarator;
}

static struct compound_statement *parse_compound_statement(struct parser *parser) {
	expect(parser->lexer, TOK_LBRACE);
	panic("parse_compound_statement ni");
}

// assume no EOF found; 
static struct external_decl_node *parse_external_decl(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_decl_specifiers(parser);
	struct declarator *declarator = parse_declarator(parser);

	// TODO: parse general declaration here, right now this function only focus on function definition

	struct compound_statement *compoundStmt = parse_compound_statement(parser);

	panic("parse_external_decl ni"); 
}

