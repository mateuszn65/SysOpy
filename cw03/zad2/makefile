all:
	gcc -Wall -o child child.c
	gcc -Wall -o main main.c
	./main 0.00006 3
	./main 0.00006 7
	./main 0.00006 20
	./main 0.00008 5
	./main 0.00006 5
	./main 0.00004 5

clean:
	rm -f child
	rm -f child.o
	rm -f main.o
	rm -f main
	rm -f pomiar_zad_2.txt
