CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g
LOAD=load_balancer
SERVER=server

.PHONY: build clean

build: tema2

tema2: main.o list.o hashtable.o $(SERVER).o $(LOAD).o 
	$(CC) $^ -o $@

list.o: list.c list.h
	$(CC) $(CFLAGS) $^ -c

hashtable.o: hashtable.c hashtable.h
	$(CC) $(CFLAGS) $^ -c

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
