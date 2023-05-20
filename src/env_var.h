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


#define SO_NAVI 25
#define SO_PORTI 4
#define SO_MERCI 6
#define SO_SIZE 10
#define SO_MIN_VITA 30
#define SO_MAX_VITA 50
#define SO_LATO 500
#define SO_SPEED 200
#define SO_CAPACITY 50
#define SO_BANCHINE 20
#define SO_FILL 100000
#define SO_LOADSPEED 10
#define SO_DAYS 20
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
