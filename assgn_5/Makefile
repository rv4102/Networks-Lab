all: queue.o mysocket.o libmsocket.a

libmsocket.a:
	ar rcs libmsocket.a queue.o mysocket.o
	rm queue.o mysocket.o

queue.o:
	gcc -c queue.c -o queue.o

mysocket.o:
	gcc -c mysocket.c -o mysocket.o

clean:
	rm -f *.o *.a