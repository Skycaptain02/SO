# dal carattere '#' fino a fine riga, il testo dentro il Makefile e`
# un commento
#
# flags per la compilazione
CFLAGS = -std=c89 -pedantic -DPROTECT_SHM

# con "make clean" solitamente si vogliono eliminare i prodotti della
# compilazione e ripristinare il contenuto originale
all: test-shm-parent test-shm-child

clean:
	rm -f *.o test-shm-parent test-shm-child *~

test-shm-parent: test-shm-parent.c test-shm.h Makefile
	gcc $(CFLAGS) test-shm-parent.c -o test-shm-parent

test-shm-child: test-shm-child.c test-shm.h Makefile
	gcc $(CFLAGS) test-shm-child.c -o test-shm-child


# il target run si usa talvolta per eseguire l'applicazione
run:  all
	./test-shm-parent
