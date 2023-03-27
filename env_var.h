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
#include <string.h>
#include <signal.h>


#define SO_NAVI 20
#define SO_PORTI 20
#define SO_MERCI 20
#define SO_SIZE 20
#define SO_MIN_VITA 20
#define SO_MAX_VITA 20
#define SO_LATO 20
#define SO_SPEED 20
#define SO_CAPACITY 20
#define SO_BANCHINE 20
#define SO_FILL 20
#define SO_LOADSPEED 20
#define SO_DAYS 20
#define SO_STORM_DURATION 20
#define SO_SWELL_DURATION 20
#define SO_MAELESTROM 20

#endif
