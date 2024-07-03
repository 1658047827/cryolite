CC = clang

CFLAGS = -Wall -std=c99

TARGET = cryolite

SRCS = $(wildcard src/*.c)

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)