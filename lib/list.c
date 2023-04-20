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
    node * temp;
    node * new_node;
    if(first == NULL){
       first = list_create(value);
    }
    else{
        temp = first;
        while(first->next != NULL){
            first = first->next;
        }
        new_node = malloc(sizeof(node));
        new_node->elem = value;
        new_node->next = NULL;
        new_node->prev = first;
        first->next = new_node;
        first = temp;
    }
    return first;
}

node * list_delete_elem(node * first, int index){
    int i, length = 0;
    node * temp;

    temp = first;
    while(first != NULL){
        length++;
        first = first->next;
    }
    first = temp;
    
    if(first != NULL && index < length){
        for(i = 0; i < index; i++){
            first = first->next;
        }
        if(index == 0){
            first->next->prev = NULL;
        }
        else if(index == length-1){
            first->prev->next = NULL;
        }
        else if(length == 1){
            return NULL;
        }
        else{
            first->prev->next = first->next;
            first->next->prev = first->prev;
        }
        
        
        free(first);
        first = temp;
        return first;
    }
    else if(index >= length || index < 0){
        printf("Error: index out of bounds\n");
        return NULL;
    }
    else{
        return first;
    }
    
}

void list_free(node * first){
    /**
     * ?
     * ?
     * ?
     * ?    COME CAZZO DEVO FARLO SONO LE 18 E NON CI STO CAPENDO UN CAZZO
     * ?
     * ?
     * ?
    */
    
}

node * list_subtract(node * first){
    node * temp;
    
    if(first != NULL){
        temp = first;
        while(first != NULL){
            first->elem.life -= 1;
            first = first->next;
        }

        first = temp;
    }
    return first;
}

node * list_get_first(node * first){
    if(first != NULL){
        while(first->prev != NULL){
            first = first->prev;
            printf("CIAO\n");
        }
    }
    return first;
}

void list_print(node * first){
    node * temp;

    if(first != NULL){
        temp = first;

        while(first != NULL){
            printf("Tipo -> %d, Peso -> %d, Vita -> %d\n", first->elem.type, first->elem.weight, first->elem.life);
            first = first->next;
        }

        first = temp;
    }
}