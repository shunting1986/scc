#include <stdio.h>
#include <sys/stat.h>

int
main(void) {
	struct stat st;
	int r;
	printf("offset __pad1 %d\n", (void *) &st.__pad1 - (void *) &st);
	printf("offset st_ino %d\n", (void *) &st.st_ino - (void *) &st);
	printf("sizeof stat %d\n", sizeof(st));
	r = stat(".", &st);

	if (r == 0) {
		printf("is dir %d\n", S_ISDIR(st.st_mode));
	} else {
		printf("fail to get stat info\n");
	}
	return 0;
}
