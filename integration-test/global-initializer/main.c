#include <stdio.h>

int a = 10;
short b = 5;
char c = 7;

int
main(void) {
	printf("a %d\n", a);
	printf("b %d\n", b);
	printf("c %d\n", c);
	return 0;
}

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
