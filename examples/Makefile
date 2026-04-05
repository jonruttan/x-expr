.POSIX:

CC?=cc
CFLAGS+=-Wall -ansi -DX_HEAP

X_EXPR_DIR=..
CFLAGS+=-I$(X_EXPR_DIR)/include
SOURCES=$(wildcard $(X_EXPR_DIR)/src/*.c)

hello: hello.c $(SOURCES)
	$(CC) $(CFLAGS) -o $@ hello.c $(SOURCES)

clean:
	rm -f hello
.PHONY: clean
