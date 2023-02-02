run:
	make compile
	./main nadawca
	./main data
	./main root@debian2 "Sample title" "Sample message"
compile:
	make clean
	gcc -Wall -o main main.c

clean:
	rm -f main
