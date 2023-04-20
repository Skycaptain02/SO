#include "list.h"

node * list_create(merci value){
    node * new_node;
    new_node = malloc(sizeof(node));
    new_node->elem = value;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

node * list_insert(node * first, merci value){
    node ** temp;
    node * new_node;
    if(first == NULL){
        printf("AO\n");
       first = list_create(value);
    }
    else{
        temp = &first;
        while(first->next != NULL){
            first = first->next;
        }
        new_node = malloc(sizeof(node));
        new_node->elem = value;
        new_node->next = NULL;
        new_node->prev = first;
        first->next = new_node;
        first = *temp;
    }
    return first;
}

void list_delete_elem(node * first, int index);
void list_free(node * first);
void list_subtract(node * first);