#ifndef _INC_SYNTREE_H
#define _INC_SYNTREE_H

#include <stdlib.h>
#include <stdio.h>
#include <inc/token.h>
#include <inc/syntree-node.h>

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;

struct syntree *syntree_init(struct translation_unit *trans_unit);

// handy function to dump the syntax tree in C
void syntree_dump(struct syntree *tree);
void syntree_destroy(struct syntree *tree);

#ifdef __cplusplus
}
#endif

#endif

