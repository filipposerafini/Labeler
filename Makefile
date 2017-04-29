TARGET = Labeller
CC = gcc
CFLAGS = -g -rdynamic $(shell pkg-config --cflags --libs opencv gtk+-3.0)
SRC = $(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f Labeller
