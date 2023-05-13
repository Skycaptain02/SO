#ifdef ENV_VAR
#else
#define ENV_VAR

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>


#define SO_NAVI 1
#define SO_PORTI 4
#define SO_MERCI 6
#define SO_SIZE 5
#define SO_MIN_VITA 1
#define SO_MAX_VITA 4
#define SO_LATO 500
#define SO_SPEED 100
#define SO_CAPACITY 10
#define SO_BANCHINE 20
#define SO_FILL 1000
#define SO_LOADSPEED 20
#define SO_DAYS 10
#define SO_STORM_DURATION 20
#define SO_SWELL_DURATION 20
#define SO_MAELESTROM 20

typedef struct merci{
    int type;
    int weight;
    int life;
}merci;

typedef struct node{
    merci elem;
    struct node * prev;
    struct node * next;
}node;

struct msgOp
{
    long type; /*= pid del porto se coda tra più porti, = 1 se coda per singolo porto*/
    int operation; /*Tipo di op da effettuare: -1 = no generico, 0 = richiesta e attracco, 1 = scarico, 2 = carico, 3 = info vita, 4 = leave*/
    int extra; /*Campo extra per valori possibili ma non sicuri (es key di ICP o lunghezza array)*/
    pid_t pid_nave; /*Campo contenente pid della nave a cui risponere*/
};
#endif
