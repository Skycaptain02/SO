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


#define SO_NAVI 0
#define SO_PORTI 10
#define SO_MERCI 6
#define SO_SIZE 5
#define SO_MIN_VITA 1
#define SO_MAX_VITA 15
#define SO_LATO 200
#define SO_SPEED 20
#define SO_CAPACITY 20
#define SO_BANCHINE 20
#define SO_FILL 1500
#define SO_LOADSPEED 20
#define SO_DAYS 5
#define SO_STORM_DURATION 20
#define SO_SWELL_DURATION 20
#define SO_MAELESTROM 20

typedef struct merci{
    int type;
    int weight;
    int life;
}merci;

#endif
