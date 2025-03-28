CC = gcc
CFLAGS = -Wall -Wextra
TARGET = cricket_server

all: $(TARGET)

$(TARGET): server.c
	$(CC) $(CFLAGS) -o $(TARGET) server.c

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run 