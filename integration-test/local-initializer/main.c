#include <stdio.h>

int
main(void) {
   const char* const statenames[20] = { "conn_listening",
                                       "conn_new_cmd",
                                       "conn_waiting",
                                       "conn_read",
                                       "conn_parse_cmd",
                                       "conn_write",
                                       "conn_nread",
                                       "conn_swallow",
                                       "conn_closing",
                                       "conn_mwrite" };

	int i;
	for (i = 0; statenames[i] != NULL; i++) {
		printf("%s\n", statenames[i]);
	}
	return 0;
}
