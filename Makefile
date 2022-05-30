guard.o: guard.cpp
	g++ -c guard.cpp

guard: guard.o
	g++ guard.o -o guard -pthread


singleton.o: singleton.cpp
	g++ -c singleton.cpp

singleton: singleton.o
	g++ singleton.o -o singleton

all: guard singleton

.PHONY: clean

clean:
	rm *.o guard singleton