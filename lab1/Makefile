TARGET=mybash
CC=gcc
CPPFLAGS=`pkg-config --cflags glib-2.0`
CFLAGS=-std=gnu11 -Wall -Wextra -Wbad-function-cast -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wno-unused-parameter -g -pedantic
LDFLAGS=`pkg-config --libs glib-2.0`
SANITIZE_FLAGS = -fsanitize=address,leak,undefined -fno-omit-frame-pointer -g
PRECOMPILED=parser.o lexer.o
LSAN_OPTIONS=suppressions=lsan.supp

# Propagar entorno a make en tests/
export CC CPPFLAGS CFLAGS LDFLAGS SANITIZE_FLAGS PRECOMPILED LSAN_OPTIONS

SOURCES=$(shell echo *.c)
SOURCES := $(filter-out obfuscated.c, $(SOURCES))
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_SAN=$(SOURCES:.c=_san.o)

# Agregar objects-arch a los directorios de busqueda de los .o precompilados
ARCHDIR=objects-$(shell uname -m)
vpath parser.o $(ARCHDIR)
vpath lexer.o $(ARCHDIR)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(PRECOMPILED) obfuscated.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

obfuscated.o: obfuscated.c
	$(CC) $(OBFUSCATED_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(OBJECTS_SAN) obfuscated.o .depend *~
	make -C tests clean

test: $(OBJECTS)
	make -C tests test

test-command: command.o
	make -C tests test-command

test-scommand: command.o
	make -C tests test-scommand

test-parsing: command.o parsing.o parser.o
	make -C tests test-parsing

test-pipeline: command.o
	make -C tests test-pipeline

memtest: $(OBJECTS)
	make -C tests memtest

# Para correr con sanitizers
%_san.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE_FLAGS) -c $< -o $@

sanitize: $(OBJECTS_SAN)
	make -C tests sanitize

sanitize-scommand-tests: command_san.o
	make -C tests sanitize-scommand-tests

sanitize-pipeline-tests: command_san.o
	make -C tests sanitize-pipeline-tests

sanitize-parsing-tests: command_san.o parsing_san.o
	make -C tests sanitize-parsing-tests

sanitize-execute-tests: builtin_san.o command_san.o execute_san.o
	make -C tests sanitize-execute-tests

sanitize-leaktest: command_san.o parsing_san.o
	make -C tests sanitize-leaktest

sanitize-all: sanitize sanitize-leaktest

.depend: $(SOURCES) obfuscated.c
	$(CC) $(CPPFLAGS) -MM $^ > $@

-include .depend

.PHONY: clean all test test-command test-scommand test-parsing test-pipeline memtest sanitize-leaktest sanitize-scommand-tests sanitize-pipeline-tests sanitize-parsing-tests sanitize-execute-tests sanitize-all
