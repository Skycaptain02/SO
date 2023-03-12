#ifndef __IPC_MSG_SMALIB_H__
#define __IPC_MSG_SMALIB_H__
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

#define MSG_LEN 120      /* 128 - sizeof(long) */

struct msgbuf {
	long mtype;             /* message type, must be > 0 */
	char mtext[MSG_LEN];    /* message data */
};

#define MY_KEY 0x123456
#define MSGTYPE_RM  7

#endif   /* __IPC_MSG_SMALIB_H__ */
