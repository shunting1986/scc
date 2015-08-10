#include <stdio.h>

typedef struct _stritem {
      struct _stritem *next;
      struct _stritem *prev;
      struct _stritem *h_next;
      int time;
      int exptime;
      int nbytes;
      unsigned short refcount;
      char nsuffix;
      char it_flags;
      char slabs_clsid;
      char nkey;
      union {
        long long cas;
        char end;
      } data[];
    } item;

struct st_type {
	char ch;
	char arr[11];
};

int 
main(void) {
	item it;
	item *ptr = &it;
	void *vp = ptr;
	printf("size %d\n", sizeof(item));
	printf("nkey off %d\n", (void *) &(ptr->nkey) - vp);
	printf("data off %d\n", (void *) &(ptr->data) - vp);
	printf("st_type size %d\n", sizeof(struct st_type));
	return 0;
}
