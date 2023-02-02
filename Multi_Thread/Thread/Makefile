CC=g++
STD=-std=c++11
CF=$(STD)


ImageFilters.out: thread.o
	$(CC) $(CF) -o ImageFilters.out thread.o

thread.o: thread.cpp
	$(CC) $(CF) -c thread.cpp -o thread.o


.PHONY: clean
clean:
	rm -rf *.o *.out output.bmp