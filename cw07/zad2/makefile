all:
	gcc -Wall -o cook cook.c -lpthread -lrt
	gcc -Wall -o supplier supplier.c -lpthread -lrt
	gcc -Wall -o pizzeria pizzeria.c -lpthread -lrt
run1:
	./pizzeria 3 3
run2:
	./pizzeria 10 10
clean:
	rm -f cook
	rm -f supplier
	rm -f pizzeria

