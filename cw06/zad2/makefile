all:
	gcc -Wall -o server server.c -lrt
	gcc -Wall -o client client.c -lrt
clean:
	rm -f server
	rm -f client
	rm -f requests.txt
runserver:
	./server
runclient:
	./client
