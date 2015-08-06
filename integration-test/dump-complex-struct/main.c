#include <stdio.h>
#include <stdlib.h>

struct context {
	char *namelist[10];
	int val;
};

int
main(void) {
	struct context *ctx = calloc(1, sizeof(*ctx));
	int i;
	for (i = 0; i < 10; i++) {
		printf("%d: %p\n", i, ctx->namelist[i]);
	}
	printf("val %d\n", ctx->val);
	return 0;
}
