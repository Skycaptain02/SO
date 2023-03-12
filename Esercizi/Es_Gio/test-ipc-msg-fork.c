#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


#define NUM_PROC  4
#define NUM_MSG  10
#define MSG_MAX_SIZE     128
#define TEST_ERROR    if (errno) {dprintf(2, \
					   "%s:%d: PID=%5d: Error %d (%s)\n",\
					   __FILE__,\
					   __LINE__,\
					   getpid(),\
					   errno,\
					   strerror(errno));}

struct my_msg_t {
	long mtype;                       /* type of message */
	char mtext[MSG_MAX_SIZE];         /* user-define message */
};

/* 
 * Personalized routine to  send messages over the  queue. It properly
 * checks many errors.
 */
int my_msg_send(int queue, const struct my_msg_t*, size_t msg_length);

/* Print status of the message queue q_id onto file descriptor fd */
static void msg_print_stats(int fd, int q_id);

int main()
{
	int q_id;
	size_t msgsize_user, msgsize_max;
	int i, j, status, num_bytes;
	struct my_msg_t mybuf;
	FILE * in_stream;
	pid_t child_pid;
	struct msqid_ds my_queue_stat;
	pid_t *snd_kids;
	pid_t rcv_pid;
	
	/* -----------------------------------------------------------
	 * SECTION (1)
	 * -----------------------------------------------------------
	 * Create queue with read/write privileges to the user, since
	 * the user must both send and receive a message
	 */
	q_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0600);
	TEST_ERROR;
	
	/* -----------------------------------------------------------
	 * SECTION (2)
	 * -----------------------------------------------------------
	 * Loop to create NUM_PROC child processes
	 */
	snd_kids = calloc(NUM_PROC, sizeof(*snd_kids));
	for (i=0; i<NUM_PROC; i++) {
		switch (child_pid = fork()) {
		case -1:
			/* Handle error */
			TEST_ERROR;
			break;
			
		case 0:
			free(snd_kids);
			/* 
			 * Child processes send NUM_MSG messages to
			 * the queue
			 */
			for(j=1; j<=NUM_MSG; j++) {
				/* 
				 * Preparing the content of the
				 * message
				 */
				num_bytes =
					sprintf(mybuf.mtext,
						"CHILD PID %5d: j=%d\n",
						getpid(), j);
				/* counting the terminating '\0' */
				num_bytes++;
				/* setting the type equal to loop count */
				mybuf.mtype = j; /* remember type > 0 */
				
				/* Now invoke my function to send */
				my_msg_send(q_id, &mybuf, num_bytes);
			}
			
			/* Done sending: child process terminates */
			exit(0);
			break;
			
		default:
			/* The parent stores the child PID */
			snd_kids[i] = child_pid;
			break;
		}
	}
	
	/*
	 * Now  the  parent  process   has  forked  all  sender  child
	 * processes.   Some   of   them   MAY   also   have   already
	 * terminated. Some  of them  MAY be  blocked because  of full
	 * queue
	 */
	
	/* Print status of the queue for debugging purpose */
	msg_print_stats(1, q_id);
	
	/* -----------------------------------------------------------
	 * SECTION (3)
	 * -----------------------------------------------------------
	 * Now forking another process to receive messages. The parent
	 * will detect when senders have finished, then will erase the
	 * queue
	 */
	if (!(rcv_pid = fork())) {
		/* Code of the receiver: always receiving */
		while (1) {
			if ((num_bytes =
			     msgrcv(q_id, &mybuf, MSG_MAX_SIZE, 0, 0)) == -1) {
				/* Error means that the queue has been
				 * erased */
				break;
			} else {
				write(1, mybuf.mtext, num_bytes);
			}
		}
		exit(0);
	}
	
	/* -----------------------------------------------------------
	 * SECTION (4)
	 * -----------------------------------------------------------
	 * Waiting  for the  sender processes  only to  terminate. The
	 * receiver will never terminate because  it is blocked on the
	 * msgrcv(...) even if the queue is empty
	*/
	for (i=0; i<NUM_PROC; i++) {
		if ((child_pid = waitpid(snd_kids[i], &status, 0)) == -1) {
			/* all sender child process are terminated */
			break;
		} else {
			dprintf(1,"PID=%d. Sender (PID=%d) terminated with status 0x%04X\n",
				getpid(),
				child_pid,
				status);
		}
	}
	/*
	 * Why can't we deallocate the queue, now that all child
	 * processes have terminated? Because, some of the messages
	 * may still be in the queue waiting of the receiver to be
	 * read
	 */
	
	/* Print status for debugging purpose */
	msg_print_stats(1, q_id);
	
	/* -----------------------------------------------------------
	 * SECTION (5)
	 * -----------------------------------------------------------
	 * Looping until  the queue  is empty. Hopefully  the receiver
	 * will do its job
	 */
	while (1) {
		/* Get the status of the queue */
		msgctl(q_id, IPC_STAT, &my_queue_stat);
		dprintf(1, "Messages left after sender termination %ld\n",
			my_queue_stat.msg_qnum);
		if (my_queue_stat.msg_qnum == 0)
			break;
		sleep(1);
	}
	
	/* -----------------------------------------------------------
	 * SECTION (6)
	 * -----------------------------------------------------------
	 * Got all messages,  then the queue can  be erased. Remember:
	 * the erasure of the queue will also unblock the receiver
	 */
	while (msgctl(q_id, IPC_RMID, NULL)) {
		TEST_ERROR;
	}
	
	/* Wait for the receiver process, then finish */
	child_pid = wait(&status);
	dprintf(1,"PID=%d. Receiver (PID=%d) terminated with status 0x%04X\n",
		getpid(),
		child_pid,
		status);
	
	exit(0);
}


int my_msg_send(int queue, const struct my_msg_t* my_msgbuf, size_t msg_length)
{
	
	/* Send the message */
	msgsnd(queue, my_msgbuf, msg_length, 0);
	switch (errno) {
	case EAGAIN:
		dprintf(2,
			"Queue is full and IPC_NOWAIT was set to have a non-blocking msgsnd()\n\
Fix it by:\n								\
  (1) making sure that some process read messages, or\n			\
  (2) changing the queue size by msgctl()\n");
		return(-1);
	case EACCES:
		dprintf(2,
	    "No write permission to the queue.\n\
Fix it by adding permissions properly\n");
		return(-1);
	case EFAULT:
		dprintf(2,
			"The address of the message isn't accessible\n");
		return(-1);
	case EIDRM:
		dprintf(2,
			"The queue was removed\n");
		return(-1);
	case EINTR:
		dprintf(2,
			"The process got unblocked by a signal, while waiting on a full queue\n");
		return(-1);
	case EINVAL:
		TEST_ERROR;
		return(-1);
	case ENOMEM:
		TEST_ERROR;
		return(-1);
	default:
		TEST_ERROR;
	}
	return(0);
}

static void msg_print_stats(int fd, int q_id) {
	struct msqid_ds my_q_data;
	int ret_val;
	
	while (ret_val = msgctl(q_id, IPC_STAT, &my_q_data)) {
		TEST_ERROR;
	}
	dprintf(fd, "--- IPC Message Queue ID: %8d, START ---\n", q_id);
	dprintf(fd, "---------------------- Time of last msgsnd: %ld\n",
		my_q_data.msg_stime);
	dprintf(fd, "---------------------- Time of last msgrcv: %ld\n",
		my_q_data.msg_rtime);
	dprintf(fd, "---------------------- Time of last change: %ld\n",
		my_q_data.msg_ctime);
	dprintf(fd, "---------- Number of messages in the queue: %ld\n",
		my_q_data.msg_qnum);
	dprintf(fd, "- Max number of bytes allowed in the queue: %ld\n",
		my_q_data.msg_qbytes);
	dprintf(fd, "----------------------- PID of last msgsnd: %d\n",
		my_q_data.msg_lspid);
	dprintf(fd, "----------------------- PID of last msgrcv: %d\n",
		my_q_data.msg_lrpid);  
	dprintf(fd, "--- IPC Message Queue ID: %8d, END -----\n", q_id);
}
