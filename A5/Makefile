all: queue.o mysocket.o libmsocket.a server.o client.o 

client.o:
	gcc main_client.c -lpthread -L. -lmsocket -o client.o

server.o:
	gcc main_server.c -lpthread -L. -lmsocket -o server.o

libmsocket.a:
	ar rcs libmsocket.a queue.o mysocket.o
	rm queue.o mysocket.o

queue.o:
	gcc -c queue.c -o queue.o

mysocket.o:
	gcc -c mysocket.c -o mysocket.o

clean:
	rm -f *.o *.a