#ifndef _INC_TYPE_H
#define _INC_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

struct type {
	int tag;
	union {
		int unused; // placeholder
	};
};

#ifdef __cplusplus
}
#endif

#endif
