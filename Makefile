# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -Wcomment -ggdb

# Define parameters
SEC_LEVEL ?= 1  # Default value is 1, can be overridden during make

# Define source files
SRC = main.c random/random.c arith.c rss.c aux.c fips202.c keccakf1600.c

# Define output executable
EXECUTABLE = main

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC)
	$(CC) $(CFLAGS) -DSEC_LEVEL=$(SEC_LEVEL) -o $@ $^

# all: main

# main: main.c random/random.c
# 	$(CC) $(CFLAGS) -o main main.c random/random.c


clean:
	rm -f $(EXECUTABLE)
