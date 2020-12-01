CPPFLAGS := -Wall -O0 -g
main: main.cpp HT.cpp HT.h sha1.h sha1.cpp Makefile
	g++ ${CPPFLAGS} main.cpp HT.cpp sha1.cpp -L. -static -l:BF_64.a -o main

clean :
	rm -rf main
	rm -rf *.o
