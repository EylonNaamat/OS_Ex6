guard.o: guard.cpp
	g++ -c guard.cpp

guard: guard.o
	g++ guard.o -o guard -pthread

main1.o: main1.c
	g++ -c main1.c

main1: main1.o
	g++ main1.o -o main1 -pthread

reactor.o: reactor.cpp
	g++ -c reactor.cpp

reactor: reactor.o
	g++ reactor.o -o reactor -pthread

singleton.o: singleton.cpp
	g++ -c singleton.cpp

singleton: singleton.o
	g++ singleton.o -o singleton

all: guard singleton reactor main1

.PHONY: clean

clean:
	rm *.o guard singleton reactor main1