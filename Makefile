all:
	gcc -Wall -c common.c
	gcc -Wall -c handlers.c
	gcc -Wall client.c handlers.o common.o -o client -lm
	gcc -Wall server.c common.o handlers.o -o server -lm

clean:
	rm common.o handlers.o client server
