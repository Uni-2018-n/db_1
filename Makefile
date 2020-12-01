CPPFLAGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
	            -Wwrite-strings -Wmissing-declarations \
		                -Wredundant-decls -Winline \
							-Wuninitialized -O0 -g
myprog: main.cpp HP.cpp HP.h HT.cpp HT.h Makefile
	g++ ${CPPFLAGS} main.cpp HP.cpp HT.cpp BF_64.a -no-pie -o main

clean :
	rm -rf myprog
	rm -rf *.o
