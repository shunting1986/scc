#ifndef _INC_PARSER_H
#define _INC_PARSER_H

#include <inc/lexer.h>
#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

struct parser;

struct parser *parser_init(struct lexer *lexer);
struct syntree *parse(struct parser *parser);
void parser_destroy(struct parser *parser);

#ifdef __cplusplus
}
#endif

#endif
