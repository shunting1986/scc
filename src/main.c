#include "util.h"

int
main(int argc, char **argv) {
	if (argc != 2) {
		panic("Usage: %s [file to compile]");
	}
	return 0;
}
