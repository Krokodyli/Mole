CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -pthread
EXEC = mole

.PHONY: clean all run debug

optimize: CFLAGS += -O2
optimize: all

all: clean
	$(CC) $(CFLAGS) list.c indexTree.c moleData.c indexer.c streamHandler.c indexingThread.c moleIO.c terminal.c main.c -o $(EXEC)

debug: CFLAGS += -ggdb
debug: all

clean:
	rm -f *.o $(EXEC)
