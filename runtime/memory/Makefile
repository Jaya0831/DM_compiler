name := memory-server

sources := $(wildcard src/*.c)
objects := $(sources:.c=.o)
headers := $(wildcard src/*.h)

mkdir_out := mkdir -p out

all: build
build: out/$(name)

clean:
	find . -type f -name *.o -delete
	rm -rf out

$(objects): %.o: %.c $(headers)
	$(CC) -c -o $@ $< $(CFLAGS)

out/$(name): $(objects)
	$(mkdir_out)
	$(CC) -o $@ $< $(LDFLAGS)
