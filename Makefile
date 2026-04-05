# # Computational Expressions in C
#
# ## Makefile
#
# @description Computational Expressions in C
# @author [Jon Ruttan](jonruttan@gmail.com)
# @copyright 2021 Jon Ruttan
# @license MIT No Attribution (MIT-0)
#
#     ., .,
#     {O,O}
#     (   )
#      " "
# Info on portable Makefiles:
# - [A Tutorial on Portable Makefiles « null program](http://nullprogram.com/blog/2017/08/20/)
# - [Makefile Assignments are Turing-Complete « null program](http://nullprogram.com/blog/2016/04/30/)
# - [os agnostic - OS detecting makefile - Stack Overflow](https://stackoverflow.com/questions/714100/os-detecting-makefile)
# - [Gagallium : Portable conditionals in makefiles](http://gallium.inria.fr/blog/portable-conditionals-in-makefiles/)
# - [make](http://pubs.opengroup.org/onlinepubs/009695399/utilities/make.html)

.POSIX:

# Override default compiler and flags
CC?=gcc
CFLAGS+=-Wall -Wextra -Wno-unused-parameter
CFLAGS+=-DX_HEAP

# Smaller, faster, flakier?
# CFLAGS+=-Wl,--gc-sections -fdata-sections -fno-stack-protector -Wa,--noexecstack -fno-builtin -fno-unwind-tables -fno-asynchronous-unwind-tables -Os

# Get the compiler name
CCOMPILER=$(CC)
ifeq ("$(CCOMPILER)", "cc")

ifeq ($(shell diff $(shell which cc) $(shell which gcc)), )
CCOMPILER=gcc
else ifeq ($(shell diff $(shell which cc) $(shell which clang)), )
CCOMPILER=clang
endif

endif

LDFLAGS?=

# Customise the settings for the compiler
ifeq ("$(CCOMPILER)", "c89")
CFLAGS+=-ansi -fdiagnostics-color=always -Wno-unused-result
DUMPMACHINE=$(shell $(CC) $(CFLAGS) -dumpmachine)
else ifeq ("$(CCOMPILER)", "c99")
CFLAGS+=-fdiagnostics-color=always -Wno-unused-result
DUMPMACHINE=$(shell $(CC) $(CFLAGS) -dumpmachine)
else ifeq ("$(CCOMPILER)", "gcc")
CFLAGS+=-ansi -fdiagnostics-color=always -Wno-unused-result
DUMPMACHINE=$(shell $(CC) $(CFLAGS) -dumpmachine)
else ifeq ("$(CCOMPILER)", "clang")
CFLAGS+=-ansi -fdiagnostics-color=always -Wno-array-bounds
DUMPMACHINE=$(shell $(CC) $(CFLAGS) -dumpmachine)
else ifeq ("$(CCOMPILER)", "tcc")
CFLAGS+=-fdiagnostics-color=always
endif

# Fallback command to use when compiler doesn't support `-dumpmachine`
ifndef DUMPMACHINE
# DUMPMACHINE=$(shell uname -m -o)
# DUMPMACHINE=$(shell echo $(shell uname -m -o) | tr A-Z a-z)
DUMPMACHINE=$(shell echo $(shell uname -m)-$(shell uname -s)-$(shell uname -o) | tr A-Z a-z)
endif

# Get the machine Target Triplet
X_MACHINE?=\"$(DUMPMACHINE)\"

# Uncomment the following, if you get "wrong interpreter" errors on OSX
#LDFLAGS+=-Wl,-no_pie

BASEDIR=.
INCDIR=$(BASEDIR)/include
SRCDIR=$(BASEDIR)/src

CFLAGS+=-I$(INCDIR)

HEADERS=$(wildcard $(INCDIR)/*.h $(INCDIR)/**/*.h)
SOURCES=$(wildcard $(SRCDIR)/*.c $(SRCDIR)/**/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=x
LIBRARY=libx-expr.a

PREFIX?=/usr/local
DESTDIR?=

# Options to be added to $(DEFS)
DEFS?=$(OSDEF) -DX_MACHINE="$(X_MACHINE)"

default: lib ## Build static library

all: $(SOURCES) $(EXECUTABLE) ## Build executable (requires a main())

debug: $(EXECUTABLE)-debug ## Build debug target

lib: $(OBJECTS) ## Build static library
	ar rcs $(LIBRARY) $(OBJECTS)

strip: $(EXECUTABLE) ## Strip symbols from target
	strip $(EXECUTABLE)

install: lib ## Install headers and library
	mkdir -p $(DESTDIR)$(PREFIX)/include/x-expr
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp $(INCDIR)/*.h $(DESTDIR)$(PREFIX)/include/x-expr/
	cp $(LIBRARY) $(DESTDIR)$(PREFIX)/lib/
.PHONY: install

uninstall: ## Remove installed headers and library
	rm -rf $(DESTDIR)$(PREFIX)/include/x-expr
	rm -f $(DESTDIR)$(PREFIX)/lib/$(LIBRARY)
.PHONY: uninstall

$(EXECUTABLE): $(OBJECTS) $(EXTRA_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(EXTRA_OBJS) $(EXTRA_LIBS) -o $@

$(EXECUTABLE)-debug: CFLAGS += -g -Og -DDEBUG
$(EXECUTABLE)-debug: LDFLAGS += -g
$(EXECUTABLE)-debug: $(EXECUTABLE)

.c.o:
	$(CC) -c $(CFLAGS) $(DEFS) -o $@ $<

lint: ## Lint sources
	$(CC) -fsyntax-only $(CFLAGS) -g -Wall -pedantic $(SOURCES)
.PHONY: lint

valgrind: ## Run Valgrind on target
	$(CC) $(CFLAGS) -g -Wall $(SOURCES) && valgrind -v --leak-check=full ./a.out && rm a.out
.PHONY: valgrind

ifndef PATH_TESTS
PATH_TESTS=tests
endif

ifndef TESTS
TESTS=$(PATH_TESTS)/src/*.spec.c
endif

test: ## Run tests
	CFLAGS="$(CFLAGS) -fno-common -g -Og -I. -DTESTS" sh $(PATH_TESTS)/test-runner/test-runner.sh $(TESTS)
.PHONY: test

test-quick: ## Run fast tests (no Valgrind)
	CFLAGS="$(CFLAGS) -fno-common -g -Og -I. -DTESTS" RUNNER=command sh $(PATH_TESTS)/test-runner/test-runner.sh $(TESTS)
.PHONY: test

coverage: ## Run tests with coverage report
	CFLAGS="$(CFLAGS) -fno-common -O0 -g --coverage -I. -DTESTS" \
		ANALYZER_FLAGS="--print-summary --txt" \
		sh $(PATH_TESTS)/test-runner/test-runner-coverage.sh $(TESTS)
.PHONY: coverage

watch: ## Watch source for changes
	while true; do \
		fswatch -o --event Created --event Updated --event MovedTo $(HEADERS) $(SOURCES) tests | \
		make debug && make test; \
	done
.PHONY: watch

doc: ## Generate C reference documentation (HTML + man pages)
	doxygen Doxyfile
.PHONY: doc

clean: ## Clean compiled files
	rm -f $(EXECUTABLE) $(EXECUTABLE)-debug *.out $(SRCDIR)/*.o $(SRCDIR)/**/*.o *.core core
	rm -f $(LIBRARY)
	rm -f $(SRCDIR)/*.gcda $(SRCDIR)/*.gcno *.gcda *.gcno
.PHONY: clean

distclean: clean ## Clean compiled files and generated docs
	rm -rf docs/ref
.PHONY: distclean

help: ## Display this help section
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z0-9_-]+:.*?## / {printf "\033[32m%-38s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
.PHONY: help
