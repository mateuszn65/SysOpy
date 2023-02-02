all:
	gcc -Wall -o catcher catcher.c
	gcc -Wall -o sender sender.c
	./catcher SIGQUEUE

clean:
	rm -f catcher
	rm -f sender
