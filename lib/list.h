#ifdef LIST
#else
#define LIST
#include <stddef.h>
#include "../src/env_var.h"

typedef struct Node{
    Merce elem;
    struct Node * next;
}Node;

typedef struct List{
    Node * top;
}List;

Node * createNode(Merce elem);
int listLength(List * list);
void listCreate(List * list);
void listInsert(List * list, Merce elem);
void listPrint(List * list);
void listSubtract(List * list, int *, int *, int);
void listRemoveToLeft(List * list, int *, int);
void listFree(List * list);

#endif