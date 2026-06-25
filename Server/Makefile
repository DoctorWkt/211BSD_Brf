CFLAGS= -O
#CFLAGS= -g -Wall -DDEBUG

brfserver: netcode.c bsdcode.c brf.h
	cc -o brfserver $(CFLAGS) netcode.c bsdcode.c

clean:
	rm -f brfserver
