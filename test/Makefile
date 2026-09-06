CC = gcc
CFLAGS =  -g -O0 -fsanitize=address -Wall -Wextra -I..
TARGET = test_agfx

SRCS = main.c ../agfx.c ../agfx_filters.c ../agfx_mask.c ../agfx_text.c ../agfx_ui.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) output*.bmp