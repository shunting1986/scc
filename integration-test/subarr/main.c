#include <stdio.h>

struct st_type {
	int ar[4];
} st;

int
main(void) {
	int *ptr;
	ptr = (int *) st.ar + 2;
	printf("%d\n", ptr - st.ar);
	return 0;
}
