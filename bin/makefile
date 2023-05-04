all: ../src/master.c
	gcc -std=c89 -Wpedantic ../src/master.c ../lib/ipc.o -o master
	gcc -std=c89 -Wpedantic ../src/navi.c ../lib/ipc.o ../lib/list.o -o navi -lm
	gcc -std=c89 -Wpedantic ../src/porti.c ../lib/ipc.o ../lib/list.o -o porti
	gcc -std=c89 -Wpedantic ../src/meteo.c ../lib/ipc.o -o meteo
	gcc -std=c89 -Wpedantic ../src/merci.c -o merci