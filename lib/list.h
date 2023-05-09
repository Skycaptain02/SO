#ifdef LIST
#else
#define LIST
#include <stddef.h>
#include "../src/env_var.h"


node * list_create(merci value);
node * list_insert(node * first, merci value);
node * list_delete_elem(node * first, int index);
void list_free(node * first);
node * list_subtract(node * first);
node * list_get_first(node * first);
void list_print(node * first, int pid);
node * list_delete_zero(node * first, int * matr_global, int riga);
int list_length(node * first);
int * list_types(node * first);
merci * list_to_array(node * first);
node * array_to_list(merci * first, int length);

#endif