all: clean compile runserver

compile:
	gcc -Wall -o server server.c -lpthread -lrt
	gcc -Wall -o client client.c -lpthread -lrt

runserver:
	./server 8002 socket
client1:
	./client Mateusz local socket
client2:
	./client Jan local socket
client1_net:
	./client Kamil net 127.0.0.1:8002
client2_net:
	./client Marek net 127.0.0.1:8002

clean:
	rm -f client
	rm -f server
