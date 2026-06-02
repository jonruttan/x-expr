.POSIX:

CC?=cc
CFLAGS+=-Wall -ansi -DX_HEAP

X_EXPR_DIR=..
CFLAGS+=-I$(X_EXPR_DIR)/include
SOURCES=$(wildcard $(X_EXPR_DIR)/src/*.c)

BUILD=build
EXAMPLES=$(BUILD)/hello $(BUILD)/gc $(BUILD)/hooks

all: $(EXAMPLES)

# Short-name aliases: `make hello`, `make gc`, `make hooks`.
hello: $(BUILD)/hello
gc: $(BUILD)/gc
hooks: $(BUILD)/hooks

$(BUILD)/hello: hello.c $(SOURCES)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ hello.c $(SOURCES)

$(BUILD)/gc: gc.c $(SOURCES)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ gc.c $(SOURCES)

$(BUILD)/hooks: hooks.c $(SOURCES)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ hooks.c $(SOURCES)

clean:
	rm -rf $(BUILD)
.PHONY: all clean hello gc hooks
