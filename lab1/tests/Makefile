# La forma normal de usar este Makefile debería ser correr
# "make test" EN EL DIRECTORIO DE ARRIBA, no en este.
CPPFLAGS+= -I..
LDFLAGS+= `pkg-config --libs check`

TARGETS=runner runner-command runner-parsing runner-scommand runner-pipeline leaktest sanitizer sanitizer_parsing sanitizer_scommand sanitizer_pipeline sanitizer_execute sanitizer_leaktest
SOURCES=$(shell echo *.c)

# Modulos que ya se compilaron
COMMON_OBJECTS=../command.o ../strextra.o
COMMON_OBJECTS_SAN=../command_san.o

ARCHDIR=objects-$(shell uname -m)

vpath parser.o ../$(ARCHDIR) ..
vpath lexer.o ../$(ARCHDIR) ..
PARSER_OBJECTS=parser.o lexer.o ../parsing.o

# Al modulo ejecutor lo recompilamos en este directorio usando mocks
MOCK_OBJECTS=builtin.o execute.o syscall_mock.o
MOCK_OBJECTS_SAN=builtin_san.o execute_san.o syscall_mock_san.o
vpath execute.c ..
vpath builtin.c ..
execute.o: CPPFLAGS += -DREPLACE_SYSCALLS=1
builtin.o: CPPFLAGS += -DREPLACE_SYSCALLS=1
execute_san.o: CPPFLAGS += -DREPLACE_SYSCALLS=1
builtin_san.o: CPPFLAGS += -DREPLACE_SYSCALLS=1


# - Cada test suite linkea lo minimo posible
# - Los runners usan la implementacion de referencia
#   de los modulos que no estan bajo prueba
runner: run_tests.o test_scommand.o test_pipeline.o test_execute.o test_parsing.o $(COMMON_OBJECTS) $(PARSER_OBJECTS) $(MOCK_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

runner-command: run_command.o test_scommand.o test_pipeline.o $(COMMON_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

runner-parsing: run_parsing.o test_parsing.o $(COMMON_OBJECTS) $(PARSER_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

runner-scommand: run_scommand.o test_scommand.o $(COMMON_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

runner-pipeline: run_pipeline.o test_pipeline.o $(COMMON_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

leaktest: leaktest.o test_scommand.o test_pipeline.o $(COMMON_OBJECTS) $(PARSER_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)


# Cada runner usa partes distintas de run_tests.c
run_tests.o:   CPPFLAGS+= -DTEST_COMMAND -DTEST_EXECUTE -DTEST_PARSER

run_command.o: CPPFLAGS+= -DTEST_COMMAND
run_command.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<


run_parsing.o: CPPFLAGS+= -DTEST_PARSER
run_parsing.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

run_scommand.o: CPPFLAGS+= -DTEST_SCOMMAND
run_scommand.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

run_pipeline.o: CPPFLAGS+= -DTEST_PIPELINE
run_pipeline.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

# Ejecutar runners
test: runner
	./$^

test-command: runner-command
	./$^

test-parsing: runner-parsing
	./$^

test-scommand: runner-scommand
	./$^

test-pipeline: runner-pipeline
	./$^

memtest: leaktest
	# Ver http://live.gnome.org/Valgrind para explicacion de G_SLICE y G_DEBUG
	# Uso el archivo glib.supp para no ver los errores internos de GLib2
	G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --leak-check=full --show-reachable=yes --suppressions=glib.supp ./$^

# Sanitize targets
sanitizer: run_tests_san.o test_scommand_san.o test_pipeline_san.o test_execute_san.o test_parsing_san.o $(COMMON_OBJECTS_SAN) $(PRECOMPILED) ../parsing_san.o $(MOCK_OBJECTS_SAN)
	$(CC) $(SANITIZE_FLAGS) -o $@ $^ $(LDFLAGS)

sanitizer_parsing: run_parsing_san.o test_parsing_san.o $(COMMON_OBJECTS_SAN) ../parsing_san.o $(PRECOMPILED)
	$(CC) $(SANITIZE_FLAGS) -o $@ $^ $(LDFLAGS)

sanitizer_scommand: run_scommand_san.o test_scommand_san.o $(COMMON_OBJECTS_SAN)
	$(CC) $(SANITIZE_FLAGS) -o $@ $^ $(LDFLAGS)

sanitizer_pipeline: run_pipeline_san.o test_pipeline_san.o $(COMMON_OBJECTS_SAN)
	$(CC) $(SANITIZE_FLAGS) -o $@ $^ $(LDFLAGS)

sanitizer_execute: run_execute_san.o test_execute_san.o $(COMMON_OBJECTS_SAN) $(MOCK_OBJECTS_SAN)
	$(CC) $(SANITIZE_FLAGS) -o $@ $^ $(LDFLAGS)

sanitizer_leaktest: leaktest_san.o test_scommand_san.o test_pipeline_san.o $(COMMON_OBJECTS_SAN) $(PRECOMPILED) ../parsing_san.o
	$(CC) $(SANITIZE_FLAGS) -o $@ $^ $(LDFLAGS)

run_tests_san.o:   CPPFLAGS+= -DTEST_COMMAND -DTEST_EXECUTE -DTEST_PARSER

run_parsing_san.o: CPPFLAGS+= -DTEST_PARSER
run_parsing_san.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE_FLAGS) -o $@ -c $<

run_scommand_san.o: CPPFLAGS+= -DTEST_SCOMMAND
run_scommand_san.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE_FLAGS) -o $@ -c $<

run_pipeline_san.o: CPPFLAGS+= -DTEST_PIPELINE
run_pipeline_san.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE_FLAGS) -o $@ -c $<

run_execute_san.o: CPPFLAGS+= -DTEST_EXECUTE
run_execute_san.o: run_tests.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE_FLAGS) -o $@ -c $<

%_san.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZE_FLAGS) -c $< -o $@

sanitize: sanitizer
	./$^

sanitize-parsing-tests: sanitizer_parsing
	./$^

sanitize-scommand-tests: sanitizer_scommand
	./$^

sanitize-pipeline-tests: sanitizer_pipeline
	./$^

sanitize-execute-tests: sanitizer_execute
	./$^

sanitize-leaktest: sanitizer_leaktest
	./$^

all: test

clean:
	rm -f $(TARGETS) *.o test*.log .depend *~

.depend: $(SOURCES)
	$(CC) $(CPPFLAGS) -MM $^ > $@

-include .depend

.PHONY: all clean test test-command test-scommand test-pipeline memtest sanitize-leaktest sanitize-scommand-tests sanitize-pipeline-tests sanitize-parsing-tests sanitize-execute-tests sanitize-all
