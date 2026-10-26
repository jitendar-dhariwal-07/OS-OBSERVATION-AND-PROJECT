CC ?= gcc
CFLAGS ?= -Wall -Wextra -pthread
BIN = tscos
SRC = src/tscos.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

run: $(BIN)
	./$(BIN)

interactive: $(BIN)
	./$(BIN) -i

test: $(BIN)
	sh tests/run_tests.sh

clean:
	rm -f $(BIN)

.PHONY: all run interactive test clean
