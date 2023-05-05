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


#define SO_NAVI 1
#define SO_PORTI 4
#define SO_MERCI 2
#define SO_SIZE 20
#define SO_MIN_VITA 1
#define SO_MAX_VITA 1
#define SO_LATO 200
#define SO_SPEED 100
#define SO_CAPACITY 20
#define SO_BANCHINE 20
#define SO_FILL 1000
#define SO_LOADSPEED 20
#define SO_DAYS 2
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

struct msgnotifica
{
    long type;
    int pid;
};


struct msgscarico {
    long type; /* type of message */
    int * merci;
};

struct msgcarico
{
    long type;
    node * list;
};




#endif
