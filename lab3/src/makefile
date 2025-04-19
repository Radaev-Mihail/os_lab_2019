CC = gcc
CFLAGS = -Wall -Wextra -I.
TARGETS = sequential_min_max parallel_min_max runner
OBJS = find_min_max.o utils.o

all: $(TARGETS)

sequential_min_max: sequential_min_max.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

parallel_min_max: parallel_min_max.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

runner: runner.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean