#include <stdio.h>

struct _IO_marker { 
	struct _IO_marker *_next; 
	struct _IO_FILE *_sbuf; 
	int _pos; 
}; 

struct _IO_FILE { 
	struct _IO_marker *_markers; 
	struct _IO_FILE *_chain;
};

int
main() {
	printf("hi\n");
	return 0;
}
