CPPFLAGS := -std=c++11 -Wall -O0 -g
main: main.cpp HT.cpp HT.h HP.cpp HP.h Makefile
	g++ ${CPPFLAGS} main.cpp  HT.cpp HP.cpp -L. -static -l:BF_64.a -o main

clean :
	rm -rf main
	rm -rf *.o
	rm -rf temp
