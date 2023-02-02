all:
	gcc -Wall -o child child.c
	gcc -Wall -o main main.c
	./main ignore
	./main handler
	./main mask
	./main pending

clean:
	rm -f main
	rm -f child
