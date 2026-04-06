.POSIX:

CC?=cc
CFLAGS+=-Wall -ansi -DX_HEAP

X_EXPR_DIR=..
CFLAGS+=-I$(X_EXPR_DIR)/include
SOURCES=$(wildcard $(X_EXPR_DIR)/src/*.c)

EXAMPLES=hello gc hooks

all: $(EXAMPLES)

hello: hello.c $(SOURCES)
	$(CC) $(CFLAGS) -o $@ hello.c $(SOURCES)

gc: gc.c $(SOURCES)
	$(CC) $(CFLAGS) -o $@ gc.c $(SOURCES)

hooks: hooks.c $(SOURCES)
	$(CC) $(CFLAGS) -o $@ hooks.c $(SOURCES)

clean:
	rm -f $(EXAMPLES)
.PHONY: all clean
