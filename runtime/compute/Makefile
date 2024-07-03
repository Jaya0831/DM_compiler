sources := $(wildcard lib/*.c)
objects := $(sources:.c=.o)
include_dir := ./include

all: libcompute.a

$(objects): %.o: %.c
	$(CC) -c -o $@ $^ -Iinclude

libcompute.a: $(objects)
	ar rcs $@ $^

.PHONY: clean

clean:
	find . -type f -name *.o -delete
	rm -f *.a
