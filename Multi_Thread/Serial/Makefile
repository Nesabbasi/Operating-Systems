CC=g++
STD=-std=c++11
CF=$(STD)


ImageFilters.out: serial.o
	$(CC) $(CF) -o ImageFilters.out serial.o

serial.o: serial.cpp
	$(CC) $(CF) -c serial.cpp -o serial.o


.PHONY: clean
clean:
	rm -rf *.o *.out output.bmp