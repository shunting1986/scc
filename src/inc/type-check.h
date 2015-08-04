#ifndef _INC_TYPE_CHECK_H
#define _INC_TYPE_CHECK_H

#include <inc/util.h>

#ifdef __cplusplus
extern "C" {
#endif

struct type;

bool type_eq(struct type *lsh, struct type *rhs);
bool type_assignable(struct type *lhs, struct type *rhs);

#ifdef __cplusplus
}
#endif

#endif
