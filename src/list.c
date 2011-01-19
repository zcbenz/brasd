#include "list.h"

#include <stdlib.h>
#include <stdio.h>

struct node *create_list() {
    struct node *head = malloc(sizeof(struct node));
    head->fd = -1;
    head->next = head->prev = NULL;

    return head;
}

void free_list(struct node *head) {
    while(head) {
        struct node *next = head->next;
        free(head);
        head = next;
    }
}

struct node *list_append(struct node *prev, int fd) {
    struct node *add = malloc(sizeof(struct node));
    add->fd    = fd;
    add->prev  = prev;
    add->next  = prev->next;
    prev->next = add;

	if(add->next)
		add->next->prev = add;

    return add;
}

int list_remove(struct node *del) {
    int fd = del->fd;
    del->prev->next = del->next;
	if(del->next)
		del->next->prev = del->prev;
    free(del);

    return fd;
}

struct node *list_find(struct node *head, int fd) {
	head = head->next;
	while(head) {
		if(head->fd == fd)
			return head;
		head = head->next;
	}

	return NULL;
}

void list_print(struct node *head) {
	head = head->next;
	while(head) {
		printf("%d, ", head->fd);
		head = head->next;
	}

	fputs("\n", stdout);
	fflush(stdout);
}
