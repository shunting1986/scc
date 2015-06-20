int scanf(const char *fmt, ...);
int printf(const char *fmt, ...);

int
main() {
	int a, b;
	int c;
	scanf("%d%d", &a, &b);
	c = a + b;
	printf("%d\n", c);
	return 0;
}
