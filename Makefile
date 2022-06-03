guard.o: guard.cpp
	g++ -c guard.cpp

guard: guard.o
	g++ guard.o -o guard -pthread

queue.o: queue.c
	g++ -c queue.c

queue: queue.o
	g++ queue.o -o queue -pthread

main1.o: main1.c
	g++ -c main1.c

main1: main1.o
	g++ main1.o -o main1 -pthread

active_object_client.o: active_object_client.cpp
	g++ -c active_object_client.cpp

active_object_client: active_object_client.o
	g++ active_object_client.o -o active_object_client -pthread

active_object_test.o: active_object_test.cpp
	g++ -c active_object_test.cpp

active_object_test: active_object_test.o
	g++ active_object_test.o -o active_object_test -pthread

selectclient.o: selectclient.cpp
	g++ -c selectclient.cpp

selectclient: selectclient.o
	g++ selectclient.o -o selectclient -pthread

reactor.o: reactor.cpp
	g++ -c reactor.cpp

reactor: reactor.o
	g++ reactor.o -o reactor -pthread

reactor_test.o: reactor_test.cpp
	g++ -c reactor_test.cpp

reactor_test: reactor_test.o
	g++ reactor_test.o -o reactor_test -pthread

singleton.o: singleton.cpp
	g++ -c singleton.cpp

singleton: singleton.o
	g++ singleton.o -o singleton

all: guard singleton reactor main1 selectclient active_object_test active_object_client queue reactor_test

.PHONY: clean

clean:
	rm *.o guard singleton reactor main1 selectclient active_object_test active_object_client queue reactor_test