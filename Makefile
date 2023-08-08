# Makefile for client.c and server.c

all: server client

server: server.c
	gcc -g -Wall server.c -o server

client: client.c
	gcc -g -Wall client.c -o client

clean:
	rm server client
