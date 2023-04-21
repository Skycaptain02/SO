#ifdef LIST
#else
#define LIST
#include <stddef.h>
#include "../src/env_var.h"

typedef struct node{
    merci elem;
    struct node * prev;
    struct node * next;
}node;


node * list_create(merci value);
node * list_insert(node * first, merci value);
node * list_delete_elem(node * first, int index);
void list_free(node * first);
node * list_subtract(node * first);
node * list_get_first(node * first);
void list_print(node * first, int pid);

#endif