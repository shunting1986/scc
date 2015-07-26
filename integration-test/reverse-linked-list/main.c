#include <stdio.h>
#include <stdlib.h>

typedef int bool;
#define true 1
#define false 0

#ifndef NULL
#define NULL 0
#endif

struct linked_list {
	struct list_item *head, *tail;
};

struct list_item {
	struct linked_list *list;
	struct list_item *next;
	int val;
};

void reverse_list(struct linked_list *list) {
	struct list_item *newhead = NULL;
	struct list_item *ptr, *next;

	for (ptr = list->head; ptr; ptr = next) {
		next = ptr->next;
		ptr->next = newhead;
		newhead = ptr;
	}
	list->tail = list->head;
	list->head = newhead;
}

void print_list(struct linked_list *list) {
	struct list_item *ptr;
	bool first = true;
	for (ptr = list->head; ptr; ptr = ptr->next) {
		if (!first) {
			printf(" ");
		}
		first = false;
		printf("%d", ptr->val);
	}
	printf("\n");
}

void free_list(struct linked_list *list) {
	struct list_item *ptr, *next;

	for (ptr = list->head; ptr; ptr = next) {
		next = ptr->next;
		free(ptr);
	}
}

int
main() {
	int N;
	while (scanf("%d", &N) != EOF && N > 0) {
		struct linked_list list;
		int i;
		list.head = list.tail = NULL;

		for (i = 0; i < N; i++) {
			struct list_item *item = malloc(sizeof(*item));
			scanf("%d", &item->val);
			item->list = &list;
			item->next = NULL;

			if (list.head == NULL) {
				list.head = list.tail = item;
			} else {
				list.tail->next = item;
				list.tail = item;
			}
		}

		// reverse
		reverse_list(&list);

		// print
		print_list(&list);

		// free
		free_list(&list);
	}
	return 0;
}
