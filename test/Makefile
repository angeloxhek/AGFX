CC = gcc
CFLAGS = -Wall -Wextra -O2 -I..
TARGET = test_agfx

SRCS = main.c ../agfx.c ../agfx_ext.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) output.bmp