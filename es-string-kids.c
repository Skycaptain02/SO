#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define NAME_EXEC "./char-loop"
#define CHAR_START "a" /* come stringa perche' deve essere terminata da zero */
#define NUM_KIDS_DEFAULT  256
/*#define SLEEP_AFTER_FORK*/


int main (int argc, char * argv[]) {
	pid_t *all_kids;
	int num_proc, i, count_signaled=0, count_exited = 0;
	int status;
	char* args[] = {NAME_EXEC, CHAR_START, NULL};
	char * s;

	/* If passed an argument to the executable */
	if (argc > 1) {
		/* If the argument is not a non-zero number */
		if (!(num_proc = atoi(argv[1]))) {
			num_proc = NUM_KIDS_DEFAULT;
		}
	} else {
		num_proc = NUM_KIDS_DEFAULT;
	}
	
	/* vector of kids PIDs */
	all_kids = malloc(num_proc*sizeof(*all_kids));

	for (i=0; i<num_proc; i++) {
		switch (all_kids[i] = fork()) {
		case -1:
			/* Handle error */
			fprintf(stderr, "%s, %d: Errore (%d) nella fork\n",
				__FILE__, __LINE__, errno);
			exit(EXIT_FAILURE);
		case 0:
			/* CHILD CODE */
			/* Free the array: not needed in child */
			free(all_kids);
			/* "Jumping" to the child executable */
			execve(NAME_EXEC, args, NULL);
			fprintf(stderr, "%s, %d: Errore (%d) nella execve\n",
				__FILE__, __LINE__, errno);
			exit(EXIT_FAILURE);
			break;
		default:
			/* PARENT CODE: nothing here */
			break;
		}
	}

#ifdef SLEEP_AFTER_FORK
	sleep(1);
#endif

	/* 
	 * Invio del segnale SIGINT (equivalente a Ctrl+C) a tutti i
	 * processi figlio. Questo segnale verra` gestito dal figlio
	 * che eseguira` una semplice exit
	 */
	for (i=0; i<num_proc; i++) {
		kill(all_kids[i], SIGINT);
	}

	/* stringa per gli exit value dei figli. +1 perche' anche 0 finale */
	s = calloc(num_proc+1,1);

	/* Reset dell'indice */
	i = 0;
	while (wait(&status) != -1) {
		if (WIFEXITED(status)) {
			count_exited++;
			/* 
			 * Memorizza carattere ritornato come exit
			 * status dal figlio
			 */
			s[i++] = WEXITSTATUS(status);
		}
		if (WIFSIGNALED(status)) {
			count_signaled++;
		}
	}
	/* Chiudi la stringa */
	s[i] = 0;

	printf("Numero figli correttamente usciti:\n%d/%d\n",
	       count_exited, num_proc);
	printf("Numero figli terminati prima di impostare l'handler di SIGINT:\n%d/%d\n",
	       count_signaled, num_proc);
	
	/*
	 * A questo punto il processo padre, avendo correttamente
	 * eseguito una wait su tutti i figli, non ha piu` figli in
	 * circolazione (terminati o in esecuzione)
	  */
	
	/* Stampa finale */
	printf("La stringa finale e` composta da %d caratteri, ed e`\n%s\n",
	       (int)strlen(s), s);
	free(all_kids);
	free(s);
	exit(EXIT_SUCCESS);
}
