TARGET = Labeler
CC = gcc
CFLAGS = -g -rdynamic `pkg-config --cflags --libs opencv gtk+-3.0`
SRC = $(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f Labeler out/tmpfile
