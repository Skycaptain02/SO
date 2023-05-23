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
#include <time.h>


#define SO_NAVI 1000
#define SO_PORTI 100
#define SO_MERCI 1
#define SO_SIZE 1
#define SO_MIN_VITA 3
#define SO_MAX_VITA 10
#define SO_LATO 1000
#define SO_SPEED 500
#define SO_CAPACITY 10
#define SO_BANCHINE 2
#define SO_FILL 500000
#define SO_LOADSPEED 10
#define SO_DAYS 10
#define SO_STORM_DURATION 20
#define SO_SWELL_DURATION 20
#define SO_MAELESTROM 20

#define PRINT_MERCI 0

typedef struct{
    int type;
    int weight;
    int life;
}Merce;

struct MsgOp{
    long type; /*= pid del porto se coda tra più porti, = 1 se coda per singolo porto*/
    int operation; /*Tipo di op da effettuare: -1 = no generico, 0 = richiesta e attracco, 1 = scarico, 2 = carico, 3 = info vita, 4 = leave*/
    int extra; /*Campo extra per valori possibili ma non sicuri (es key di ICP o lunghezza array)*/
    pid_t pid_nave; /*Campo contenente pid della nave a cui risponere*/
};
#endif
