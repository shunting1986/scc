#include <stdio.h>

struct pt {
	int x, y;
	int z;
} a;

struct pt b;

int
main() {
	struct pt c;

	while (scanf("%d%d%d%d%d%d", &a.x, &a.y, &a.z, &b.x, &b.y, &b.z) != -1) {
		// x1 i + y1 j + z1 k
		// x2 i + y2 j + z2 k
		c.x = a.y * b.z - a.z * b.y;
		c.y = a.z * b.x - a.x * b.z;
		c.z = a.x * b.y - a.y * b.x;
		printf("%d %d %d\n", c.x, c.y, c.z);
	}

	return 0;
}
