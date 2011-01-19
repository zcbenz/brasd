#ifndef LIST_H
#define LIST_H

#include <event.h>

struct node {
    int fd;
    struct event event;
    struct node *next;
    struct node *prev;
};

struct node *create_list();
void free_list(struct node*);
struct node *list_append(struct node*, int);
int list_remove(struct node*);
struct node *list_find(struct node*, int);
void list_print(struct node*);

#endif /* end of LIST_H */
