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
    length = list_length(first);
    
    if(temp != NULL && index < length){
        for(i = 0; i < index; i++){
            temp = temp->next;
        }
        if(index == 0){
            temp->next->prev = NULL;
        }
        else if(index == length-1){
            temp->prev->next = NULL;
        }
        else if(length == 1){
            return NULL;
        }
        else{
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
        }
        free(temp);
        return list_get_first(first);
    }
    else if(index >= length || index < 0){
        printf("Error: index out of bounds\n");
        return NULL;
    }
    else{
        return NULL;
    }
    
}

void list_free(node * first){
    node * next;

    while(first != NULL){
        next = first->next;
        free(first);
        first = next;
    }
    free(next);
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
        }
    }
    return first;
}

node * list_delete_zero(node * first){
    node * temp;
    int i = 0, length;
    
    length = list_length(first);
    temp = first;
    while(temp->next != NULL){
        if(temp->elem.life == 0){
            if(i == 0){
                temp->next->prev = NULL;
            }
            else if(length == 1){
                return NULL;
            }
            else{
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
            }
            temp = temp->next;
            length--;
            printf("len: %d\n", length);
            continue;
        }
        i++;
        temp = temp->next;
    }
    
    if(temp->elem.life == 0){
        temp->prev->next = NULL;
        length--;
        temp = temp->prev;
    }
    printf("i = %d, length = %d\n", i, length);
    first = list_get_first(temp);
    free(temp);
    return first;
}
   

node * list_delete_current(node * current){
    
}

void list_print(node * first, int pid){

    if(first != NULL){
        first = list_get_first(first);

        while(first != NULL){
            printf("Tipo -> %d, Peso -> %d, Vita -> %d, PID -> %d\n", first->elem.type, first->elem.weight, first->elem.life, pid);
            first = first->next;
        }

        first = list_get_first(first);
    }
}

int list_length(node * first){
    int length = 0;
    node * temp;
    if(first != NULL){
        temp = first;
        while(temp != NULL){
            length++;
            temp = temp->next;
        }   
    }
    return length;
}