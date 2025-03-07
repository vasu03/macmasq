opt =-O3 -Wall -std=c2x

all: clean macmasq

macmasq: macmasq.o
	gcc ${opt} $^ -o $@

macmasq.o: macmasq.c
	gcc ${opt} -c $^

clean:
	rm -f macmasq *.o