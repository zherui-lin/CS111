CC = gcc $(ARCH)
CFLAGS = -ggdb -Wall -Werror -m32

all: main-static

main-static: main.o math.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f main-static *.o *.s *~ .*~

.PHONY: all clean
