CFLAGS=-std=c11 -g -static

k8cc: k8cc.c

test: k8cc
		./test.sh

clean:
		rm -f k8cc *.o *~ tmp*

.PHONY: test clean
