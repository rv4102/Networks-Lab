all: client server

client:
	gcc main_client.c mysocket.c queue.c -o client.o -lpthread

server:
	gcc main_Server.c mysocket.c queue.c -o server.o -lpthread

clean:
	rm -f *.o