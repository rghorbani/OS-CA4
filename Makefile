all: lib.o MemManager.o PageInfo.o ThreadFunctions.o
	g++ -o FileScramble main.cpp lib.o MemManager.o PageInfo.o ThreadFunctions.o -lpthread
lib.o: lib.cpp lib.h
	g++ -c lib.cpp
MemManager.o: MemManager.cpp MemManager.h PageInfo.h
	g++ -c MemManager.cpp
PageInfo.o: PageInfo.cpp PageInfo.h
	g++ -c PageInfo.cpp
ThreadFunctions.o: ThreadFunctions.cpp ThreadFunctions.h MemManager.h lib.h PageInfo.h
	g++ -c ThreadFunctions.cpp

clean:
	rm -rf *.o
clear:
	rm -rf *.o FileScramble
