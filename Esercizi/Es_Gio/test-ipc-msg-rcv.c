#include "ipc-msg-common.h"

/*
 * argv[1] is the type of message to receive
 *
 * content of the message is written to stdout
 */
int main(int argc, char * argv[])
{
	int q_id;
	struct msgbuf my_msg;
	int num_bytes;
	long rcv_type;

	if (argc <= 1) {
		fprintf(stderr, "Must specify an integer to select the type of message to receive\n");
		return(-1);
	}

	/*
	 * Create a message queue if it does not exist. Just get its
	 * ID if it exists already. The key MY_KEY is shared via a
	 * common #define
	 */
	q_id = msgget(MY_KEY, IPC_CREAT | 0600);
	TEST_ERROR;

	rcv_type = atol(argv[1]);
	printf("PID=%d, receiving message of type %ld\n", getpid(), rcv_type);
	while (1) {
		/* now receiving the message */
		num_bytes = msgrcv(q_id, &my_msg, MSG_LEN, rcv_type, 0);
		if (num_bytes >= 0) {
			/* received a good message (possibly of zero length) */
			break;
		}

		/* now handling errors */
		if (errno == EINTR) {
			fprintf(stderr, "(PID=%d): interrupted by a signal while waiting for a message of type %ld on Q_ID=%d. Trying again\n",
				getpid(), rcv_type, q_id);
			continue;
		}
		if (errno == EIDRM) {
			printf("The Q_ID=%d was removed. Let's terminate\n", q_id);
			exit(0);
		}		
	}

	printf("(PID=%d): Q_id=%d: msg type=%ld \"%s\" RECEIVED\n",
	       getpid(), q_id, my_msg.mtype, my_msg.mtext);

	/* Check if special message received to erase the queue */
	if (my_msg.mtype == MSGTYPE_RM) {
		printf("Now removing the queue\n");
		msgctl(q_id, IPC_RMID, NULL);
	}

	return(0);
}
