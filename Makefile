CPPFLAGS := -Wall -O0 -g
myprog: main.cpp HT.cpp HT.h Makefile
	g++ ${CPPFLAGS} main.cpp HT.cpp -L. -static -l:BF_64.a -o myprog

clean :
	rm -rf myprog
	rm -rf *.o
