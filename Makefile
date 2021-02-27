CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

k8cc: $(OBJS)
		$(CC) -o k8cc $(OBJS) $(LDFLAGS)

$(OBJS): k8cc.h

test: k8cc
		./test.sh

clean:
		rm -f k8cc *.o *~ tmp*

build:
		gcc -o k8cc ./k8cc.c && ./k8cc "12 + 34 - 5"

.PHONY: test clean
