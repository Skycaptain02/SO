master: ./src/master.c
	gcc -std=c89 -Wpedantic ./src/master.c ./lib/ipc.o -o ./bin/master

navi: navi.c
	gcc -std=c89 -Wpedantic ./src/navi.c ./lib/ipc.o -o ./bin/navi

porti: porti.c
	gcc -std=c89 -Wpedantic ./src/porti.c ./lib/ipc.o -o ./bin/porti

meteo: meteo.c
	gcc -std=c89 -Wpedantic ./src/meteo.c ./lib/ipc.o -o ./bin/meteo

merci: merci.c
	gcc -std=c89 -Wpedantic ./src/merci.c ./lib/ipc.o -o ./bin/merci
	
all: ./src/master.c
	gcc -std=c89 -Wpedantic ./src/master.c ./lib/ipc.o -o ./bin/master
	gcc -std=c89 -Wpedantic ./src/navi.c ./lib/ipc.o -o ./bin/navi
	gcc -std=c89 -Wpedantic ./src/porti.c ./lib/ipc.o ./lib/list.o -o ./bin/porti
	gcc -std=c89 -Wpedantic ./src/meteo.c ./lib/ipc.o -o ./bin/meteo
	gcc -std=c89 -Wpedantic ./src/merci.c -o ./bin/merci