master: master.c
	gcc -std=c89 -Wpedantic master.c -o master

navi: navi.c
	gcc -std=c89 -Wpedantic navi.c -o navi

porti: porti.c
	gcc -std=c89 -Wpedantic porti.c -o porti

meteo: meteo.c
	gcc -std=c89 -Wpedantic meteo.c -o meteo
	
all: master.c navi.o
	gcc -std=c89 -Wpedantic master.c -o master
	gcc -std=c89 -Wpedantic navi.c -o navi
	gcc -std=c89 -Wpedantic porti.c -o porti
	gcc -std=c89 -Wpedantic meteo.c -o meteo