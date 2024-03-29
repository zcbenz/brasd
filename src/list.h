#ifndef LIST_H
#define LIST_H

#include "sds.h"

struct node {
    int fd;
    sds buffer;
    struct node *next;
    struct node *prev;
};

struct node *create_list();
struct node *list_append(struct node*, int);
struct node *list_find(struct node*, int);
void free_list(struct node*);
void list_print(struct node*);
int list_remove(struct node*);

#endif /* end of LIST_H */
