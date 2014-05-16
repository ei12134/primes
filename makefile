CC = gcc
PROG = primes
CFLAGS = -Wall -lrt
SRCS = primes.c circularqueue.c
BIN_DIR = bin

all: bin monitor

bin:
	mkdir -p ${BIN_DIR}

monitor:
	$(CC) $(SRCS) -o $(BIN_DIR)/$(PROG) $(CFLAGS)

clean:
	@rm -f $(BIN_DIR)/$(PROG) $(BIN_DIR)/*.o core