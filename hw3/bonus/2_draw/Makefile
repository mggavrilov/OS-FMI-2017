ifndef CC
	CC=gcc
endif
CFLAGS=-D_POSIX_C_SOURCE=199309L -std=c99 -Werror -Wall -Wpedantic -Wextra
SRCS=main.c
OBJS=$(subst .c,.o,$(SRCS))
RM=rm -f

foo: main.o
	$(CC) $(CFLAGS) -o main main.c -lm

all: main

clean:
	$(RM) $(OBJS) main

