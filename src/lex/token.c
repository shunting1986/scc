#include <stdio.h>
#include <stdlib.h>

#include <inc/token.h>

void token_destroy(union token token) {
	switch (token.token_tag) {
	case TOK_INT: case TOK_LPAREN: case TOK_RPAREN: case TOK_LBRACE:
	case TOK_RBRACE: case TOK_COMMA: case TOK_SEMICOLON:
		break;
	case TOK_IDENTIFIER:
		free(token.id_token.s);
		break;
	default:
		panic("token_destroy %d ni", token.token_tag);
		break;
	}
}

void token_dump(union token token) {
	switch (token.token_tag) {
	case TOK_INT:
		printf("[int]\n");
		break;
	case TOK_IDENTIFIER:
		printf("[id]: %s\n", token.id_token.s);
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
	default:
		panic("token_dump %d ni", token.token_tag);
	}
}


