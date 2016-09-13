CC = clang
CFLAGS = -Wall -lrt -lm -pthread
PROG = primes
BIN_DIR = bin
SRC_DIR = src
SRCS = ${SRC_DIR}/primes.c ${SRC_DIR}/circularqueue.c

all: bin primes

bin:
	mkdir -p ${BIN_DIR}

primes:
	$(CC) $(SRCS) -o $(BIN_DIR)/$(PROG) $(CFLAGS)

clean:
	@rm -f $(BIN_DIR)/$(PROG) $(BIN_DIR)/*.o core
