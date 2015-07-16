#ifndef _INC_PARSER_H
#define _INC_PARSER_H

#include <inc/lexer.h>
#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

struct parser {
	struct lexer *lexer;
};

struct parser *parser_init(struct lexer *lexer);
void parser_destroy(struct parser *parser);

// parse methods
struct syntree *parse(struct parser *parser);

// stmt
struct syntreebasenode *parse_statement(struct parser *parser);
struct compound_statement *parse_compound_statement(struct parser *parser);

// decl
int initiate_declaration(union token tok);
struct declaration *parse_declaration(struct parser *parser);
int initiate_type_name(union token tok);

// expr
struct expression *parse_expression(struct parser *parser);
struct conditional_expression *parse_conditional_expression(struct parser *parser);
#define parse_constant_expression parse_conditional_expression
struct assignment_expression *parse_assignment_expression(struct parser *parser);

#ifdef __cplusplus
}
#endif

#endif
