#ifdef LIST
#else
#define LIST
#include <stddef.h>
#include "../src/env_var.h"


node * list_create(merci);
node * list_insert(node *, merci);
node * list_delete_elem(node *, int);
void list_free(node *);
node * list_subtract(node *);
node * list_get_first(node *);
void list_print(node *);
node * list_delete_zero(node *, int *, int *, int);
int list_length(node *);
int * list_types(node *);
merci * list_to_array(node *);
node * array_to_list(merci *, int);
node * list_remove_elem(node *, int, int *);

#endif