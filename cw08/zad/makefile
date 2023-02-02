compile:
	gcc -Wall -o main main.c -lpthread -lrt
numbers:
	./main 4 numbers in/lena.ascii.pgm out/lena.ascii.pgm
block:
	./main 4 block in/lena.ascii.pgm out/lena.ascii.pgm
test_threads_no:
	./main 1 numbers in/lena.ascii.pgm out/lena.n1.ascii.pgm
	./main 1 block in/lena.ascii.pgm out/lena.b1.ascii.pgm
	./main 2 numbers in/lena.ascii.pgm out/lena.n2.ascii.pgm
	./main 2 block in/lena.ascii.pgm out/lena.b2.ascii.pgm
	./main 4 numbers in/lena.ascii.pgm out/lena.n4.ascii.pgm
	./main 4 block in/lena.ascii.pgm out/lena.b4.ascii.pgm
	./main 8 numbers in/lena.ascii.pgm out/lena.n8.ascii.pgm
	./main 8 block in/lena.ascii.pgm out/lena.b8.ascii.pgm
	./main 16 numbers in/lena.ascii.pgm out/lena.n16.ascii.pgm
	./main 16 block in/lena.ascii.pgm out/lena.b16.ascii.pgm
test_image_size:
	./main 4 block in/tracks.ascii.pgm out/tracks.ascii.pgm
	./main 4 block in/lena.ascii.pgm out/lena.ascii.pgm
	./main 4 block in/gator.ascii.pgm out/gator.ascii.pgm
test: clean compile test_image_size test_threads_no
clean:
	rm -f main
	rm -f times.txt
	rm -f out/*
