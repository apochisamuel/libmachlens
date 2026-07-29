CC ?= clang
CXX ?= clang++
AR ?= ar
CFLAGS ?= -O2 -g -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -Isrc
LDFLAGS ?=

LIB := libmachlens.a
SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
TEST_SOURCES := $(filter-out tests/test_support.c,$(wildcard tests/test_*.c))
TEST_BINS := $(TEST_SOURCES:.c=)
TOOL := tools/machlens

.PHONY: all clean test tools strict sanitize

all: $(LIB) $(TOOL)

$(LIB): $(OBJECTS)
	$(AR) rcs $@ $^

src/%.o: src/%.c src/machlens.h src/internal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TOOL): tools/machlens.c $(LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ tools/machlens.c $(LIB)

tests/test_%: tests/test_%.c tests/test_support.c tests/test_support.h $(LIB)
	$(CC) $(CPPFLAGS) -Itests $(CFLAGS) $(LDFLAGS) -o $@ $< tests/test_support.c $(LIB)

test: $(TEST_BINS)
	@set -e; for test_bin in $(TEST_BINS); do echo "[test] $$test_bin"; "./$$test_bin"; done

tools: $(TOOL)

strict:
	$(MAKE) clean
	$(MAKE) CFLAGS='-O2 -g -std=c11 -Wall -Wextra -Wpedantic -Werror' all test

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS='-O1 -g -std=c11 -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer' LDFLAGS='-fsanitize=address,undefined' test


clean:
	rm -f ./*.o src/*.o $(LIB) $(TEST_BINS) $(TOOL)
