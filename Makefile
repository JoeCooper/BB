CC=gcc
CFLAGS=-I./src -Wall -Werror

# The dependencies directory "./src/"
DEPS = src/hashtable.h
FORGE_TARGET = forge
FORGE__OBJ = hashtable.o forge.o encoding.o model.o
FORGE_OBJ = $(patsubst %,src/%,$(FORGE__OBJ))

all: $(FORGE_TARGET)

$(FORGE_TARGET): $(FORGE_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

src/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f src/*.o forge