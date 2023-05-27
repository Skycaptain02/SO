#include "list.h"

void listCreate(List * list){
    list->top = NULL;
}

Node * createNode(Merce elem){
    Node * newNode = malloc(sizeof(Node *));
    newNode->elem = elem;
    newNode->next = NULL;
    return newNode;
}

void listInsert(List * list, Merce elem){
    Node * newNode, * temp;
    newNode = createNode(elem);
    if(list->top == NULL){
        list->top = newNode;
        return;
    }
    temp = list->top;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = newNode;
}

void listPrint(List * list){
    Node * temp = list->top;
    if(temp == NULL){
        printf("LA LISTA E' VUOTA pid: %d\n", getpid());
        return;
    }
    while(temp != NULL){
        printf("Type: %d Life: %d Weight: %d, pid: %d\n",temp->elem.type, temp->elem.life, temp->elem.weight, getpid());
        temp = temp->next;
    }
}

void listSubtract(List * list, int * merce_scaduta, int * statusMerci, int caller){
    Node * head = list->top;
    Node * prev = NULL;
    while(head != NULL){
        head->elem.life = head->elem.life - 1;
        if(head->elem.life <= 0){
            * merce_scaduta = * merce_scaduta + 1;
            if(caller){ /*Porto*/
                statusMerci[((head->elem.type - 1) * 5) + 3] += 1;
                statusMerci[((head->elem.type - 1) * 5)] -= 1;
            }else{ /*Nave*/
                statusMerci[((head->elem.type - 1) * 5) + 4] += 1;
                statusMerci[((head->elem.type - 1) * 5) + 1] -= 1;
            }
            if(prev == NULL){
                list->top = head->next;
            }
            else{
                prev->next = head->next;
            }
        }
        else{
            prev = head;
        }
        head = head->next;
    }
}

void listRemoveToLeft(List * list, int * rem_life, int elem_type){
    Node * head = list->top;
    Node * prevNode = NULL;
    Node * temp;
    while(head != NULL){
        if(head->elem.type == elem_type){
            if(rem_life != NULL){
                * rem_life = head->elem.life;
            }
            if(prevNode == NULL){
                list->top = head->next;
            }
            else{
                prevNode->next = head->next;
            }
            return;
        }
        else{
            prevNode = head;
            head = head->next;
        }
    }
}

void listFree(List * list){
    Node * head = list->top;
    Node * next;

    while(head != NULL){
        next = head->next;
        free(head);
        head = next;
    }
    list->top = head;
}

int listLength(List * list){
    int length = 0;
    Node * temp = list->top;
    while(temp != NULL){
        length++;
        temp = temp->next;
    }
    return length;
}