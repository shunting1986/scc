#include <stdio.h>
#include <stdlib.h>

#include <inc/token.h>

void token_destroy(union token token) {
	switch (token.tok_tag) {
	case TOK_INT: case TOK_LPAREN: case TOK_RPAREN: case TOK_LBRACE:
	case TOK_RBRACE: case TOK_COMMA: case TOK_SEMICOLON: case TOK_AMPERSAND:
	case TOK_ASSIGN: case TOK_ADD: case TOK_RETURN: case TOK_CONSTANT_VALUE:
		break;
	case TOK_IDENTIFIER:
		free(token.id.s);
		break;
	case TOK_STRING_LITERAL:
		free(token.str.s);
		break;
	default:
		panic("token_destroy %d ni", token.tok_tag);
		break;
	}
}

void token_dump(union token token) {
	switch (token.tok_tag) {
	case TOK_INT:
		printf("[int]\n");
		break;
	case TOK_RETURN:
		printf("[return]\n");
		break;
	case TOK_IDENTIFIER:
		printf("[id]: %s\n", token.id.s);
		break;
	case TOK_LPAREN:
		printf("'('\n");
		break;
	case TOK_RPAREN:
		printf("')'\n");
		break;
	case TOK_LBRACE:
		printf("'{'\n");
		break;
	case TOK_RBRACE:
		printf("'}'\n");
		break;
	case TOK_COMMA:
		printf("','\n");
		break;
	case TOK_SEMICOLON:
		printf("';'\n");
		break;
	case TOK_AMPERSAND:
		printf("'&'\n");
		break;
	case TOK_ASSIGN:
		printf("'='\n");
		break;
	case TOK_ADD:
		printf("'+'\n");
		break;
	case TOK_STRING_LITERAL:
		printf("[string_literal] %s\n", token.str.s);
		break;
	case TOK_CONSTANT_VALUE:
		printf("[const_val] %d\n", token.const_val.ival);
		break;
	default:
		panic("token_dump %d ni", token.tok_tag);
	}
}


