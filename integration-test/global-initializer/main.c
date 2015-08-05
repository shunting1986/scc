#include <stdio.h>

// basic variables
int a = 10;
short b = 5;
char c = 7;

void test_basic_variables() {
	printf("a %d\n", a);
	printf("b %d\n", b);
	printf("c %d\n", c);
}

// int array with implicit size
int iarr_nosize[] = {
	2, 3, 5
};

// int array with explicit size
int iarr_withsize[10] = {
	16, 8, 4, 2, 1
}; 

void test_int_arr() {
	int i;
	printf("iarr_nosize:");
	for (i = 0; i < sizeof(iarr_nosize) / sizeof(iarr_nosize[0]); i++) {
		printf(" %d", iarr_nosize[i]);
	}
	printf("\n");

	printf("iarr_withsize:");
	for (i = 0; i < sizeof(iarr_withsize) / sizeof(iarr_withsize[0]); i++) {
		printf(" %d", iarr_withsize[i]);
	}
	printf("\n");
}

// single structure
struct item {
	int first;
	int second, third;
	char buf[64];
};

struct item single = {3, 4};

void test_structure() {
	printf("single: first %d, second %d, thrid %d\n", single.first, single.second, single.third);
}

// advanced structure
struct adv_item {
	const char *name;
	void (*ptr)(void);
	short ss;
	int junk1;
	char arr[6];
};

struct adv_item adv_single = {"hello", NULL, 3};
struct adv_item null_single = { NULL, NULL };

void test_advanced_structure() {
	printf("adv single: name %s, ptr %p, ss %d, junk1 %d, arr[0] %d\n", adv_single.name, adv_single.ptr, adv_single.ss, adv_single.junk1, adv_single.arr[0]);
	printf("null_single: name %p, ptr %p\n", null_single.name, null_single.ptr);
}

// structure array
struct adv_item adv_list[] = {
	{ "First", NULL, 3},
	{ "Second", NULL},
	{ "Last", NULL },
	{ NULL, NULL},
};

void test_structure_array() {
	struct adv_item *p;
	printf("struct list:");
	// for (p = adv_list; p->name; p++) {
	for (p = adv_list; p->name; ++p) {
		printf(" %s", p->name);
	}
	printf("\n");
}

int
main(void) {
	test_basic_variables();
	test_int_arr();
	test_structure();
	test_advanced_structure();
	test_structure_array();
	return 0;
}

// TODO char array using {} or ""

#if 0

// copied from mongoose
struct ssl_func {
  const char *name;   // SSL function name
  void  (*ptr)(void); // Function pointer
	short ss;
	int junk1;
	char arr[6];
};

int
main(void) {

struct ssl_func ssl_sw[] = {
  {"SSL_free",   NULL, 3},
  {"SSL_accept",   NULL},
  {"SSL_connect",   NULL},
  {"SSL_read",   NULL},
  {"SSL_write",   NULL},
  {"SSL_get_error",  NULL},
  {"SSL_set_fd",   NULL},
  {"SSL_new",   NULL},
  {"SSL_CTX_new",   NULL},
  {"SSLv23_server_method", NULL},
  {"SSL_library_init",  NULL},
  {"SSL_CTX_use_PrivateKey_file", NULL},
  {"SSL_CTX_use_certificate_file",NULL},
  {"SSL_CTX_set_default_passwd_cb",NULL},
  {"SSL_CTX_free",  NULL},
  {"SSL_load_error_strings", NULL},
  {"SSL_CTX_use_certificate_chain_file", NULL},
  {NULL,    NULL}
};


	struct ssl_func *p;
	for (p = ssl_sw; p->name; p++) {
		printf("name %s\n", p->name);
	}
	return 0;
}

#endif
