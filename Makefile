CPPFLAGS := -Wall -O0 -g
main: main.cpp HT.cpp HT.h  Makefile
	g++ ${CPPFLAGS} main.cpp  HT.cpp -L. -static -l:BF_64.a -o main

clean :
	rm -rf main
	rm -rf *.o
