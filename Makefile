all: build

build: source.o TuringMachine.o
	gcc -Wall $^ -o task1

source.o: source.c
	gcc -Wall -c -g $^

TuringMachine.o: TuringMachine.c
	gcc -Wall -c -g $^

run:
	./task1

.PHONY: clean

clean:
	rm -fr *.o task1
