#include "ipc-msg-common.h"

/*
 * argv[1] is the type of message to send;
 *
 * content of the message is read from stdin. If the length of the
 * content exceeds the buffer the message is truncated
 */
int main(int argc, char * argv[])
{
	int q_id;
	struct msgbuf my_msg;

	if (argc <= 1) {
		fprintf(stderr, "Must specify an integer >0 as message type\n");
		return(-1);
	}

	/*
	 * Create a message queue if it does not exist. Just get its
	 * ID if it exists already. The key MY_KEY is shared via a
	 * common #define
	 */
	q_id = msgget(MY_KEY, IPC_CREAT | 0600);
	TEST_ERROR;

	/* Constructing the message */
	my_msg.mtype = atol(argv[1]);
	if (my_msg.mtype <= 0) {
		fprintf(stderr, "Must specify an integer >0 as message type\n");
		return(-1);
	}
	printf("Enter the text to send below:\n");
	fgets(my_msg.mtext, MSG_LEN, stdin);

	/* now sending the message */
	msgsnd(q_id, &my_msg, MSG_LEN, 0);
	printf("(PID=%d): Q_id=%d: msg type=%ld \"%s\" SENT\n",
	       getpid(), q_id, my_msg.mtype, my_msg.mtext);

	return(0);
}
