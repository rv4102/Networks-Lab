all: client server

client: 
	gcc main_client.c mysocket.c queue.c -o client

server:
	gcc main_server.c mysocket.c queue.c -o server

clean:
	rm -f client server