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

void test_int_arr() {
	int i;
	printf("iarr_nosize:");
	for (i = 0; i < sizeof(iarr_nosize) / sizeof(iarr_nosize[0]); i++) {
		printf(" %d", iarr_nosize[i]);
	}
	printf("\n");
}

int
main(void) {
	test_basic_variables();
	test_int_arr();
	return 0;
}

// int array with explicit size
// TODO basic int arr with unspecified size
// TODO char array using {} or ""
// TODO structure (handle unspecified fields)
// TODO structure array (with specified or unspecified size) (handle unspecified field) (handle extra item, clear them to 0);

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