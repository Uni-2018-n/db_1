CPPFLAGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
	            -Wwrite-strings -Wmissing-declarations \
		                -Wredundant-decls -Winline \
							-Wuninitialized -O0 -g
myprog: main.cpp HP.cpp HP.h
	g++ ${CPPFLAGS} main.cpp HP.cpp BF_64.a -no-pie -o myprog
